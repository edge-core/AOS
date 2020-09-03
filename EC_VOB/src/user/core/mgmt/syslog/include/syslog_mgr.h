/* Module Name: SYSLOG_MGR.H
 * Purpose: Initialize the resources and provide som function
 *          for the system log module.
 *
 * Notes:
 *
 * History:
 *    10/17/01       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


#ifndef SYSLOG_MGR_H
#define SYSLOG_MGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "syslog_om.h"
#include "sys_cpnt.h"
#include "sys_module.h"
#include "syslog_type.h"

/* NAME CONSTANT DECLARATIONS
 */
#define MESSAGE_INDEX_1     "%s create task fail."
#define MESSAGE_INDEX_2     "%s switch to default."
#define MESSAGE_INDEX_3     "Allocate %s memory fail."
#define MESSAGE_INDEX_4     "Free %s memory fail."
#define MESSAGE_INDEX_5     "%s task idle too long."
#define MESSAGE_INDEX_6     "Unit %ld, fan %ld fail."
#define MESSAGE_INDEX_7     "%s function fails."
#define MESSAGE_INDEX_8     "System coldStart notification."
#define MESSAGE_INDEX_9     "System warmStart notification."
#define MESSAGE_INDEX_10    "Unit %ld, Port %2ld link-up %s notification."
#define MESSAGE_INDEX_11    "Unit %ld, Port %2ld link-down notification."
#define MESSAGE_INDEX_12    "Trunk %ld link-up notification."
#define MESSAGE_INDEX_13    "Trunk %ld link-down notification."
#define MESSAGE_INDEX_14    "VLAN %ld link-up notification."
#define MESSAGE_INDEX_15    "VLAN %ld link-down notification."
#define MESSAGE_INDEX_16    "Authentication failure notification."
#define MESSAGE_INDEX_17    "STA become new root bridge notification."
#define MESSAGE_INDEX_18    "STA topology change happened on %s."
#define MESSAGE_INDEX_19    "RMON rising alarm, index %lu, value %lu."
#define MESSAGE_INDEX_20    "RMON falling alarm, index %lu, value %lu."
#define MESSAGE_INDEX_21    "Unit %ld, %s power change to %s."
#define MESSAGE_INDEX_22    "Device over heat."
#define MESSAGE_INDEX_23    "Device MAC Address oversize."

#ifdef FTTH_OKI    /* FTTH_OKI */
#define MESSAGE_INDEX_24    "Blade %ld %s interface card insert by %s."
#define MESSAGE_INDEX_25    "Blade %ld Port %2ld UMC power down."
#define MESSAGE_INDEX_26    "Blade %ld Port %2ld UMC power up."
#define MESSAGE_INDEX_27    "Blade %ld Port %2ld Loopback start."
#define MESSAGE_INDEX_28    "Blade %ld Port %2ld Loopback not test."
#define MESSAGE_INDEX_29    "Blade %ld Port %2ld Loopback OAM setting timeout."
#define MESSAGE_INDEX_30    "Blade %ld Port %2ld Loopback LB error timeout."
#define MESSAGE_INDEX_31    "Blade %ld Port %2ld Loopback OAM release timeout."
#define MESSAGE_INDEX_32    "Blade %ld Port %2ld Loopback normal end."
#define MESSAGE_INDEX_33    "Blade %ld Port %2ld Loopback busy."
#define MESSAGE_INDEX_34    "Blade %ld Port %2ld Loopback stop."
#define MESSAGE_INDEX_35    "Blade %ld Port %2ld Loopback UMC timeout."
#define MESSAGE_INDEX_36    "Blade %ld %s diagnosis start."
#define MESSAGE_INDEX_37    "Blade %ld %s diagnosis result %s."
#define MESSAGE_INDEX_38    "Fan %ld fault."
#define MESSAGE_INDEX_39    "Fan %ld fault restoration."
#define MESSAGE_INDEX_40    "Blade %ld Port %2ld UMC optical low input power."
#define MESSAGE_INDEX_41    "Blade %ld Port %2ld UMC optical low input power restoration."
#define MESSAGE_INDEX_42    "Blade %ld Port %2ld UMC UTP port link down."
#define MESSAGE_INDEX_43    "Blade %ld Port %2ld UMC UTP port link up."
#define MESSAGE_INDEX_44    "Blade %ld Port %2ld UMC media converter fault."
#define MESSAGE_INDEX_45    "Blade %ld Port %2ld UMC media converter fault restoration."
#define MESSAGE_INDEX_46    "Blade %ld Port %2ld port link down."
#define MESSAGE_INDEX_47    "Blade %ld Port %2ld port link up."
#define MESSAGE_INDEX_48    "Blade %ld %s interface card fault."
#define MESSAGE_INDEX_49    "Blade %ld %s interface card fault restoration."
#define MESSAGE_INDEX_50    "Blade %ld Port %2ld port link change alarm sensitivity event %s."
#define MESSAGE_INDEX_51    "Blade %ld Port %2ld UMC UTP port link change alarm sensitivity event %s."
#define MESSAGE_INDEX_52    "Blade %ld Port %2ld UMC UTP low input power alarm sensitivity event %s."
#define MESSAGE_INDEX_53    "Blade %ld Port %2ld UMC media converter fault alarm sensitivity event %s."
#define MESSAGE_INDEX_54    "Fan %ld Fan alarm sensitivity event %s."
#define MESSAGE_INDEX_55    "Blade %ld %s Interface card alarm sensitivity event %s."
#endif  /* END_OF_FTTH_OKI */

#define MESSAGE_INDEX_56    "Unit %ld, Port %ld have intrusion mac address %s."
#define MESSAGE_INDEX_57    "Loopback test failure."
#define MESSAGE_INDEX_58    "Fan Recover, unit=[%ld], fanIndex=[%ld]."
#define MESSAGE_INDEX_59    "Ipfilter reject, mode=[%ld]; ip=[%d.%d.%d.%d]."
#define MESSAGE_INDEX_63    "DHCP request failed - will retry later."
#define MESSAGE_INDEX_64    "DHCP server responded."
#define MESSAGE_INDEX_65    "SMTP connect host %s failure."

#if (SYS_CPNT_POE == TRUE)
#define MESSAGE_INDEX_66    "Unit %lu, PSE port %lu status: %s."
#define MESSAGE_INDEX_67    "Unit %lu, Peth main power usage on:%lu Watt."
#define MESSAGE_INDEX_68    "Unit %lu, Peth main power usage off:%lu Watt."
#endif  /* SYS_CPNT_POE */

#define MESSAGE_INDEX_81    "%s"
#define MESSAGE_INDEX_82    "%s"
#define MESSAGE_INDEX_83    "MainBoardVerMismatch:%s."
#define MESSAGE_INDEX_84    "ModuleVerMismatch:%s."
#define MESSAGE_INDEX_85    "ModuleInsertion:%s."
#define MESSAGE_INDEX_86    "ModuleRemoval:%s."
#define MESSAGE_INDEX_87    "TCN reason:%s."

#if (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_32)
#define MESSAGE_INDEX_88    "Secure address %s learned on unit %ld port %ld."
#define MESSAGE_INDEX_89    "Security violation on unit %ld port %ld, address %s."
#define MESSAGE_INDEX_90    "Authentication failure on unit %ld port %ld, user %s."
#define MESSAGE_INDEX_91    "Logon on unit %ld port %ld, user %s."
#define MESSAGE_INDEX_92    "Logoff on unit %ld port %ld, user %s"
#endif

#if(SYS_CPNT_VRRP == TRUE)
#define MESSAGE_INDEX_93    "VRRP_VM_ recieve packet DA error"
#define MESSAGE_INDEX_94    "VRRP_VM_ recieve packet rifNum error"
#define MESSAGE_INDEX_95    "VRRP_VM_ L_MEM_Allocate error"
#define MESSAGE_INDEX_96    "VRRP_VM_ L_MREF_GetPdu error"
#define MESSAGE_INDEX_97    "VRRP_VM_ miscofiguration"
#define MESSAGE_INDEX_98    "VRRP_VM_ get primary rifNum error"
#define MESSAGE_INDEX_99    "VRRP_VM_ get primary rifInfo error"
#define MESSAGE_INDEX_100   "VRRP_TXRX_ SendAdvertisement L_MEM_Allocate error"
#define MESSAGE_INDEX_101   "VRRP_TXRX_ L_MREF_Constructor error"
#define MESSAGE_INDEX_102   "VRRP_TXRX_ get primary rifNum error"
#define MESSAGE_INDEX_103   "VRRP_TXRX_ get primary rifInfo error"
#endif

#if (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_32)
#define MESSAGE_INDEX_104   "Test Trap notification."
#define MESSAGE_INDEX_105   "Ip Forwarding is changed by mstp"
#endif

#if (SYS_CPNT_LLDP == TRUE)
#define MESSAGE_INDEX_106   "LLDP remote tables changed."
#define MESSAGE_INDEX_257   "LLDP remote table changed on %s."
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
#define MESSAGE_INDEX_143    "Unit %ld, Port %ld: %s"
#define MESSAGE_INDEX_144    "Unit %ld, Port %ld: %s"
#endif  /* #if (SYS_CPNT_EFM_OAM == TRUE) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
#define MESSAGE_INDEX_145   "BPDU guard shuts down port %lu."
#endif

#if (SYS_CPNT_LLDP_MED == TRUE)
#define MESSAGE_INDEX_153   "LLDP-MED Detect, Type %lu,ID %s,Class %lu."
#endif

#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
#define MESSAGE_INDEX_155    "CPU rising trap."
#define MESSAGE_INDEX_156    "CPU falling trap."
#define MESSAGE_INDEX_157    "Memory rising trap."
#define MESSAGE_INDEX_158    "Memory falling trap"
#endif /* #if (SYS_CPNT_SYSMGMT_RESMIB == TRUE) */

#define MESSAGE_INDEX_183   "Reject %s %s."

#if (SYS_CPNT_ATC_STORM == TRUE)
#define MESSAGE_INDEX_160    "ATC broadcast storm alarm on port %ld"
#define MESSAGE_INDEX_161    "ATC broadcast storm clear on port %ld"
#define MESSAGE_INDEX_162    "ATC broadcast traffic_control gets enabled on port %ld."
#define MESSAGE_INDEX_163    "ATC broadcast traffic_control are released on port %ld."

#define MESSAGE_INDEX_164    "ATC multicast storm alarm on port %ld"
#define MESSAGE_INDEX_165    "ATC multicast storm clear on port %ld"
#define MESSAGE_INDEX_166    "ATC multicast traffic_control gets enabled on port %ld."
#define MESSAGE_INDEX_167    "ATC multicast traffic_control are released on port %ld."
#endif

#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
#define MESSAGE_INDEX_168    "Unit %ld, port %2ld %s."
#endif /* #if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */

#if (SYS_CPNT_DAI == TRUE)
#define MESSAGE_INDEX_169    "DAI drop:IP:%s,MAC:%s,V:%ld,P:%ld"
#endif

#if(SYS_CPNT_CFM==TRUE)
#define MESSAGE_INDEX_170    "RMPID %lu in MA %lu MD %lu is discovered."
#define MESSAGE_INDEX_171    "RMPID %lu in MA %lu MD %lu is down."
#define MESSAGE_INDEX_172   "MEP receives a CCM with existing MPID %lu in MA %lu MD %lu."
#define MESSAGE_INDEX_173   "MPID %lu in MA %lu MD %lu receives its own CCMs."
#define MESSAGE_INDEX_174   "MPID %lu in MA %lu MD %lu receives an unexpected CCM."
#define MESSAGE_INDEX_175   "RMPID %lu in MA %lu MD %lu is missing."
#define MESSAGE_INDEX_176   "All expected remote MEPs are up in MA %lu MD %lu."
#define MESSAGE_INDEX_177   "MPID %lu has detected the fault with %s."
#endif  /* #if(SYS_CPNT_CFM==TRUE) */

#if (SYS_CPNT_XFER_AUTO_UPGRADE== TRUE)
#define MESSAGE_INDEX_178    "Upgrade: %s; %s."
#endif

/* DHCP client sends a trap when receiving a packet from a rogue server. */
#define MESSAGE_INDEX_186   "DHCPSNP ATK:%s,IP:%s,MAC:%s"

#define MESSAGE_INDEX_190   "STP port state: MSTID %lu, %s."

#if (SYS_CPNT_DHCPV6SNP == TRUE)
#define MESSAGE_INDEX_192    "DHCP6SNP attack: MAC:%s,V:%ld,P:%ld"
#endif

#if (SYS_CPNT_DHCP_INFORM == TRUE)
#define MESSAGE_INDEX_193   "DHCP Inform request failed on VLAN %ld - will retry later."
#define MESSAGE_INDEX_194   "DHCP Inform server respond on VLAN %ld."
#endif

#define MESSAGE_INDEX_195   "MAJOR_ALARM_ Power module is failed"
#define MESSAGE_INDEX_196   "MAJOR_ALARM_ Power module is recovered"
#define MESSAGE_INDEX_197   "MINOR_ALARM_ Thermal is overheat"

#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
#define MESSAGE_INDEX_198    "SFP TX power high alarm on port %ld"
#define MESSAGE_INDEX_199    "SFP TX power low alarm on port %ld"
#define MESSAGE_INDEX_200    "SFP TX power high warning on port %ld"
#define MESSAGE_INDEX_201    "SFP TX power low warning on port %ld"
#define MESSAGE_INDEX_202    "SFP RX power high alarm on port %ld"
#define MESSAGE_INDEX_203    "SFP RX power low alarm on port %ld"
#define MESSAGE_INDEX_204    "SFP RX power high warning on port %ld"
#define MESSAGE_INDEX_205    "SFP RX power low warning on port %ld"
#define MESSAGE_INDEX_206    "SFP Temperature high alarm on port %ld"
#define MESSAGE_INDEX_207    "SFP Temperature low alarm on port %ld"
#define MESSAGE_INDEX_208    "SFP Temperature high warning on port %ld"
#define MESSAGE_INDEX_209    "SFP Temperature low warning on port %ld"
#define MESSAGE_INDEX_210    "SFP Voltage high alarm on port %ld"
#define MESSAGE_INDEX_211    "SFP Voltage low alarm on port %ld"
#define MESSAGE_INDEX_212    "SFP Voltage high warning on port %ld"
#define MESSAGE_INDEX_213    "SFP Voltage low warning on port %ld"
#define MESSAGE_INDEX_214    "SFP Current high alarm on port %ld"
#define MESSAGE_INDEX_215    "SFP Current low alarm on port %ld"
#define MESSAGE_INDEX_216    "SFP Current high warning on port %ld"
#define MESSAGE_INDEX_217    "SFP Current low warning on port %ld"
#endif

#define MESSAGE_INDEX_218   "MAJOR_ALARM_ All fans are failed"
#define MESSAGE_INDEX_219   "MAJOR_ALARM_ Fan is recovered"
#define MESSAGE_INDEX_220   "MINOR_ALARM_ Fan is failed"
#define MESSAGE_INDEX_221   "The alarm input %lu exists"
#define MESSAGE_INDEX_222   "MAJOR_ALARM_ Power module set is wrong"
#define MESSAGE_INDEX_223   "MAJOR_ALARM_ Power module set is correct"

#if (SYS_CPNT_BGP == TRUE)
#define MESSAGE_INDEX_224   "BGP: %s"
#define MESSAGE_INDEX_225   "BGP established, ip: %s, last err: 0x%04x, state: %s"
#define MESSAGE_INDEX_226   "BGP backward trans, ip: %s, last err: 0x%04x, state: %s"
#endif

#if (SYS_CPNT_CRAFT_PORT == TRUE)
#define MESSAGE_INDEX_228   "Unit %ld, Craft Port link-up notification."
#define MESSAGE_INDEX_229   "Unit %ld, Craft Port link-down notification."
#endif

#define MESSAGE_INDEX_232   "STA root bridge is changed to %s."

#if (SYS_CPNT_VRRP == TRUE)
/* new vrrp state change log message index */
#define MESSAGE_INDEX_233   "VRRP operation state change to %s."
#endif

#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
#define MESSAGE_INDEX_234    "SFP TX power alarm warning cease on port %ld"
#define MESSAGE_INDEX_235    "SFP RX power alarm warning cease on port %ld"
#define MESSAGE_INDEX_236    "SFP Temperature alarm warning cease on port %ld"
#define MESSAGE_INDEX_237    "SFP Voltage alarm warning cease on port %ld"
#define MESSAGE_INDEX_238    "SFP Current alarm warning cease on port %ld"
#endif

#if(SYS_CPNT_SYNCE == TRUE)
#define MESSAGE_INDEX_245   "SyncE ESMC Rx on Eth %s, status/SSM %s/%s"
#define MESSAGE_INDEX_246   "SyncE clock source on Eth %s"
#endif

#define MESSAGE_INDEX_247    "Unit %ld, active power is changed to %s."

#define MESSAGE_INDEX_248    "%s, authentication failure."
#define MESSAGE_INDEX_249    "%s, authentication success."
#define MESSAGE_INDEX_250    "%s, login."
#define MESSAGE_INDEX_251    "%s, logout."

#define MESSAGE_INDEX_252    "%s%s"

#define MESSAGE_INDEX_253    "%s"

#define MESSAGE_INDEX_254    "Create user %s."
#define MESSAGE_INDEX_255    "Delete user %s."
#define MESSAGE_INDEX_256    "Modify user %s privilege level to %lu."

#define MESSAGE_INDEX_258    "STA TC received on %s, Bridge %02X-%02X-%02X-%02X-%02X-%02X."

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
#define MESSAGE_INDEX_259    "CPU guard control trap."
#define MESSAGE_INDEX_260    "CPU guard release trap."
#endif
#define MESSAGE_INDEX_264     "%s"
#if (SYS_CPNT_HTTP == TRUE)
#define MESSAGE_INDEX_266    "%s"
#endif

#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP == TRUE)
#define MESSAGE_INDEX_267   "Config file %s is overwritten by dynamic provision."
#define MESSAGE_INDEX_268   "Dynamic provision starts to provision with config file %s."
#define MESSAGE_INDEX_269   "Dynamic provision fails because of invalid option %u."
#define MESSAGE_INDEX_270   "Dynamic provision fails because server name cannot be resolved."
#define MESSAGE_INDEX_271   "Dynamic provision fails to download config file %s."
#endif /* SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP */

#define SYSLOG_MGR_ELLIPSIS_STR    "..."
#define SYSLOG_MGR_USER_INFO_STR_LEN_MAX   128
#define SYSLOG_MGR_FILE_COPY_FILE_TYPE_STR_LEN_MAX   10
#define SYSLOG_MGR_FILE_COPY_SOURCE_OPERATION_STR_LEN_MAX   80
#define SYSLOG_MGR_FILE_COPY_DESTINATION_OPERATION_STR_LEN_MAX   80
#define SYSLOG_MGR_FILE_COPY_RESULTE_STR_LEN_MAX   10
#define SYSLOG_MGR_FILE_COPY_MESSAGE_STR_LEN_MAX   14 + \
                                                   SYSLOG_MGR_USER_INFO_STR_LEN_MAX + \
                                                   SYSLOG_MGR_FILE_COPY_FILE_TYPE_STR_LEN_MAX + \
                                                   SYSLOG_MGR_FILE_COPY_SOURCE_OPERATION_STR_LEN_MAX + \
                                                   SYSLOG_MGR_FILE_COPY_DESTINATION_OPERATION_STR_LEN_MAX + \
                                                   SYSLOG_MGR_FILE_COPY_RESULTE_STR_LEN_MAX

#define SYSLOG_MGR_GET_MSGBUFSIZE(field_name)                       \
            (SYSLOG_MGR_MSGBUF_TYPE_SIZE +                        \
            sizeof(((SYSLOG_MGR_IPCMsg_T*)0)->data.field_name))

#define SYSLOG_MGR_MSGBUF_TYPE_SIZE sizeof(union SYSLOG_MGR_IpcMsg_Type_U)

#define SYSLOG_MGR_FACILITY_TO_STR(facility,facility_str) \
    {   switch(facility) \
        { \
            case 16: \
                strcpy(facility_str, "Local use 0"); \
                break; \
            case 17: \
                strcpy(facility_str, "Local use 1"); \
                break; \
            case 18: \
                strcpy(facility_str, "Local use 2"); \
                break; \
            case 19: \
                strcpy(facility_str, "Local use 3"); \
                break; \
            case 20: \
                strcpy(facility_str, "Local use 4"); \
                break; \
            case 21: \
                strcpy(facility_str, "Local use 5"); \
                break; \
            case 22: \
                strcpy(facility_str, "Local use 6"); \
                break; \
            case 23: \
                strcpy(facility_str, "Local use 7"); \
                break; \
            default: \
                strcpy(facility_str, "Invalid facility value"); \
                break; \
        } \
    }


#define SYSLOG_MGR_LEVEL_TO_STR(level,level_str) \
    {   switch(level) \
        { \
            case 0: \
                strcpy(level_str, "System unusable"); \
                break; \
            case 1: \
                strcpy(level_str, "Immediate action needed"); \
                break; \
            case 2: \
                strcpy(level_str, "Critical conditions"); \
                break; \
            case 3: \
                strcpy(level_str, "Error conditions"); \
                break; \
            case 4: \
                strcpy(level_str, "Warning conditions"); \
                break; \
            case 5: \
                strcpy(level_str, "Normal but significant condition"); \
                break; \
            case 6: \
                strcpy(level_str, "Informational messages only"); \
                break; \
            case 7: \
                strcpy(level_str, "Debugging messages"); \
                break; \
            default: \
                strcpy(level_str, "Invalid level value"); \
                break; \
        } \
    }

/*
#define SYSLOG_MGR_GET_MSGBUFSIZE(struct_name) \
        (SYSLOG_MGR_MSGBUF_TYPE_SIZE + sizeof(struct struct_name));
  */
/* DATA TYPE DECLARATIONS
 */
enum
{
    CREATE_TASK_FAIL_MESSAGE_INDEX = 1,         /* (string) */
    SWITCH_TO_DEFAULT_MESSAGE_INDEX = 2,        /* (string) */
    ALLOCATE_MEMORY_FAIL_MESSAGE_INDEX = 3,     /* (string) */
    FREE_MEMORY_FAIL_MESSAGE_INDEX = 4,         /* (string) */
    TASK_IDLE_TOO_LONG_MESSAGE_INDEX = 5,       /* (string) */
    FAN_FAIL_MESSAGE_INDEX = 6,                 /* (ui32_t, ui32_t) */
    FUNCTION_RETURN_FAIL_INDEX = 7,             /* (string) */
    SYSTEM_COLDSTART_MESSAGE_INDEX = 8,         /* void */
    SYSTEM_WARMSTART_MESSAGE_INDEX = 9,         /* void */
    NORMAL_PORT_LINK_UP_MESSAGE_INDEX = 10,     /* (ui32_t, ui32_t) */
    NORMAL_PORT_LINK_DOWN_MESSAGE_INDEX = 11,   /* (ui32_t, ui32_t) */
    TRUNK_PORT_LINK_UP_MESSAGE_INDEX = 12,      /* (ui32_t) */
    TRUNK_PORT_LINK_DOWN_MESSAGE_INDEX = 13,    /* (ui32_t) */
    VLAN_LINK_UP_MESSAGE_INDEX = 14,            /* (ui32_t) */
    VLAN_LINK_DOWN_MESSAGE_INDEX = 15,          /* (ui32_t) */
    AUTHENTICATION_FAILURE_MESSAGE_INDEX = 16,  /* void */
    STA_ROOT_CHANGE_MESSAGE_INDEX = 17,         /* void */
    STA_TOPOLOGY_CHANGE_MESSAGE_INDEX = 18,     /* void */
    RMON_RISING_ALARM_MESSAGE_INDEX = 19,       /* void */
    RMON_FALLING_ALARM_MESSAGE_INDEX = 20,      /* void */
    POWER_STATUS_CHANGE_MESSAGE_INDEX = 21,     /* (ui32_t, ui32_t, ui32_t) */
    OVER_HEAT_TRAP_INDEX = 22,                  /* void */
    MAC_ADDRESS_MESSAGE_INDEX = 23,             /* void */

#ifdef FTTH_OKI    /* FTTH_OKI */
    INTERFACE_CARD_COLD_START_MESSAGE_INDEX = 24,
    UMC_POWER_DOWN_MESSAGE_INDEX = 25,
    UMC_POWER_UP_MESSAGE_INDEX = 26,
    LOOPBACK_TEST_START_MESSAGE_INDEX = 27,
    LOOPBACK_NOT_TEST_MESSAGE_INDEX = 28,
    LOOPBACK_OAM_SETTING_TIMEOUT_MESSAGE_INDEX = 29,
    LOOPBACK_LBERROR_TIMEOUT_MESSAGE_INDEX = 30,
    LOOPBACK_OAM_RELEASE_TIMEOUT_MESSAGE_INDEX = 31,
    LOOPBACK_NORMAL_END_MESSAGE_INDEX = 32,
    LOOPBACK_BUSY_MESSAGE_INDEX = 33,
    LOOPBACK_STOP_MESSAGE_INDEX = 34,
    LOOPBACK_UMC_TIMEOUT_MESSAGE_INDEX = 35,
    DIAGNOSIS_START_MESSAGE_INDEX = 36,
    DIAGNOSIS_RESULT_MESSAGE_INDEX = 37,
    FAN_FAILURE_MESSAGE_INDEX = 38,
    FAN_RECOVER_MESSAGE_INDEX = 39,
    UMC_LIP_MESSAGE_INDEX = 40,
    UMC_LIP_RECOVER_MESSAGE_INDEX = 41,
    UMC_LINKDOWN_MESSAGE_INDEX = 42,
    UMC_LINKUP_MESSAGE_INDEX = 43,
    UMC_MC_FAULT_MESSAGE_INDEX = 44,
    UMC_MC_FAULT_RECOVER_MESSAGE_INDEX = 45,
    PORT_LINK_DOWN_MESSAGE_INDEX = 46,
    PORT_LINK_UP_MESSAGE_INDEX = 47,
    HARDWARE_FAULT_MESSAGE_INDEX = 48,
    HARDWARE_FAULT_RECOVER_MESSAGE_INDEX = 49,
    LINK_CHANGE_EVENT_MESSAGE_INDEX = 50,
    UMC_UTP_LINK_EVENT_MESSAGE_INDEX = 51,
    UMC_LIP_EVENT_MESSAGE_INDEX = 52,
    UMC_MC_FAULT_EVENT_MESSAGE_INDEX = 53,
    FAN_EVENT_MESSAGE_INDEX = 54,
    HARDWARE_FAULT_EVENT_MESSAGE_INDEX = 55,
#endif  /* END_OF_FTTH_OKI */

    PORT_SECURITY_TRAP_INDEX = 56,
    LOOPBACK_TEST_FAILURE_MESSAGE_INDEX = 57, /*void*/
    FAN_RECOVERY_MESSAGE_INDEX = 58, /*ui32_t,ui32_t*/
    MGMT_IP_FLT_REJECT_MESSAGE_INDEX = 59, /*ui32_t,ui8_t,ui8_t,ui8_t,ui8_t*/
    DHCP_IP_RETRIEVE_FAILURE_INDEX = 63,
    DHCP_IP_RETRIEVE_SUCCESS_INDEX = 64,
    SMTP_CONN_FAILURE_MESSAGE_INDEX = 65,

#if (SYS_CPNT_POE == TRUE)
    PETH_PSE_PORT_ON_OFF_MESSAGE_INDEX = 66,
    PETH_MAIN_POWER_USAGE_ON_MESSAGE_INDEX = 67,
    PETH_MAIN_POWER_USAGE_OFF_MESSAGE_INDEX = 68,
#endif

    THERMAL_RISING_MESSAGE_INDEX = 81,
    THERMAL_FALLING_MESSAGE_INDEX =82,
    MAIN_BOARD_VER_MISMATCH_MESSAGE_INDEX = 83,
    MODULE_VER_MISMATCH_MESSAGE_INDEX = 84,
    MODULE_INSERTION_MESSAGE_INDEX = 85,
    MODULE_REMOVAL_MESSAGE_INDEX = 86,
    TCN_MESSAGE_INDEX = 87,

#if (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_32)
    SECURE_ADDRESS_LEARNED_INDEX = 88,
    SECURE_VIOLATION2_INDEX = 89,
    SECURE_LOGIN_FAILURE_INDEX = 90,
    SECURE_LOGON_INDEX = 91,
    SECURE_LOGOFF_INDEX = 92,
#endif

#if(SYS_CPNT_VRRP == TRUE)
    VRRP_VM_RECEIVE_PACKET_DA_ERROR_INDEX = 93,
    VRRP_VM_RECEIVE_PACKET_RIFNUM_ERROR_INDEX = 94,
    VRRP_VM_L_MEM_ALLOCATE_ERROR_INDEX = 95,
    VRRP_VM_L_MREF_GETPDU_ERROR_INDEX = 96,
    VRRP_VM_MISCONFIGURATION_INDEX = 97,
    VRRP_VM_GET_PRIMARY_RIFNUM_ERROR_INDEX = 98,
    VRRP_VM_GET_PRIMARY_RIFINFO_ERROR_INDEX = 99,
    VRRP_TXRX_SENDADVERTISEMENT_L_MEM_ALLOCATE_ERROR_INDEX = 100,
    VRRP_TXRX_L_MREF_CONSTRUCTOR_ERROR_INDEX = 101,
    VRRP_TXRX_GET_PRIMARY_RIFNUM_ERROR_INDEX = 102,
    VRRP_TXRX_GET_PRIMARY_RIFINFO_ERROR_INDEX = 103,
#endif

#if (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_32)
    TEST_TRAP_INDEX = 104,
    IP_FORWARDING_CHANGED_BY_MSTP_TRAP_INDEX = 105,
#endif

#if(SYS_CPNT_LLDP == TRUE)
    LLDP_REM_TABLE_CHANGED_INDEX = 106,
#endif

#if (SYS_CPNT_DAI == TRUE)
    DAI_DROP_ARP_PACKET_MESSAGE_INDEX = 169,
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
    DOT3_OAM_THRESHOLD_MESSAGE_INDEX = 143,
    DOT3_OAM_NON_THRESHOLD_MESSAGE_INDEX = 144,
#endif  /* #if (SYS_CPNT_EFM_OAM == TRUE) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    STP_BPDU_GUARD_PORT_SHUTDOWN_MESSAGE_INDEX = 145,
#endif

#if (SYS_CPNT_LLDP_MED == TRUE)
    LLDP_MED_TOPOLOGY_CHANGE_DETECTED_INDEX = 153,
#endif

#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
    CPU_RAISE_MESSAGE_INDEX = 155,
    CPU_FALLING_MESSAGE_INDEX = 156,
    MEMORY_RAISE_EVENT_MESSAGE_INDEX = 157,
    MEMORY_FALLING_EVENT_MESSAGE_INDEX = 158,
#endif /* #if (SYS_CPNT_SYSMGMT_RESMIB == TRUE) */

#if (SYS_CPNT_ATC_STORM == TRUE)
    BCAST_STORM_ALARM_FIRE_INDEX =160,
    BCAST_STORM_ALARM_CLEAR_INDEX =161,
    BCAST_STORM_TC_APPLY_INDEX =162,
    BCAST_STORM_TC_RELEASE_INDEX =163,

    MCAST_STORM_ALARM_FIRE_INDEX =164,
    MCAST_STORM_ALARM_CLEAR_INDEX =165,
    MCAST_STORM_TC_APPLY_INDEX =166,
    MCAST_STORM_TC_RELEASE_INDEX =167,
#endif/* #if(SYS_CPNT_ATC_STORM==TRUE) */

#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
    NETWORK_ACCESS_PORT_LINK_DETECTION_INDEX= 168,
#endif

#if(SYS_CPNT_CFM == TRUE)
    CFM_MEP_UP_MESSAGE_INDEX = 170,
    CFM_MEP_DOWN_MESSAGE_INDEX = 171,
    CFM_CONFIG_FAIL_MESSAGE_INDEX = 172,
    CFM_LOOP_FIND_MESSAGE_INDEX = 173,
    CFM_MEP_UNKNOWN_MESSAGE_INDEX = 174,
    CFM_MEP_MISSING_MESSAGE_INDEX = 175,
    CFM_MA_UP_MESSAGE_INDEX = 176,
    CFM_FAULT_ALARM_MESSAGE_INDEX = 177,
#endif  /* #if(SYS_CPNT_CFM==TRUE) */

#if(SYS_CPNT_XFER_AUTO_UPGRADE==TRUE)
    XFER_AUTO_UPGRADE_MESSAGE_INDEX=178,
#endif

    MGMT_IP_FLT_INET_REJECT_MESSAGE_INDEX = 183,
    /*   DHCP client sends a trap when receiving a packet from a rogue server. */
    DHCP_ROGUE_SERVER_ATTACK_MESSAGE_INDEX = 186,   /* (ui8_t, ui8_t) */

    XSTP_PORT_STATE_CHANGE_MESSAGE_INDEX = 190,

#if (SYS_CPNT_DHCPV6SNP == TRUE)
    DHCPV6SNP_DROP_BOGUS_SERVER_PACKET_MESSAGE_INDEX = 192,
#endif

#if (SYS_CPNT_DHCP_INFORM == TRUE)
    DHCP_INFORM_RETRIEVE_FAILURE_INDEX = 193,
    DHCP_INFORM_RETRIEVE_SUCCESS_INDEX = 194,
#endif
    MAJOR_ALARM_POWER_STATUS_CHANGED_INDEX = 195,
    MAJOR_ALARM_POWER_STATUS_RECOVER_INDEX = 196,
    MINOR_ALARM_THERMAL_OVERHEAT_INDEX = 197,

#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    SFP_TX_POWER_HIGH_ALARM_MESSAGE_INDEX = 198,
    SFP_TX_POWER_LOW_ALARM_MESSAGE_INDEX = 199,
    SFP_TX_POWER_HIGH_WARNING_MESSAGE_INDEX = 200,
    SFP_TX_POWER_LOW_WARNING_MESSAGE_INDEX = 201,
    SFP_RX_POWER_HIGH_ALARM_MESSAGE_INDEX = 202,
    SFP_RX_POWER_LOW_ALARM_MESSAGE_INDEX = 203,
    SFP_RX_POWER_HIGH_WARNING_MESSAGE_INDEX = 204,
    SFP_RX_POWER_LOW_WARNING_MESSAGE_INDEX = 205,
    SFP_TEMP_HIGH_ALARM_MESSAGE_INDEX = 206,
    SFP_TEMP_LOW_ALARM_MESSAGE_INDEX = 207,
    SFP_TEMP_HIGH_WARNING_MESSAGE_INDEX = 208,
    SFP_TEMP_LOW_WARNING_MESSAGE_INDEX = 209,
    SFP_VOLTAGE_HIGH_ALARM_MESSAGE_INDEX = 210,
    SFP_VOLTAGE_LOW_ALARM_MESSAGE_INDEX = 211,
    SFP_VOLTAGE_HIGH_WARNING_MESSAGE_INDEX = 212,
    SFP_VOLTAGE_LOW_WARNING_MESSAGE_INDEX = 213,
    SFP_CURRENT_HIGH_ALARM_MESSAGE_INDEX = 214,
    SFP_CURRENT_LOW_ALARM_MESSAGE_INDEX = 215,
    SFP_CURRENT_HIGH_WARNING_MESSAGE_INDEX = 216,
    SFP_CURRENT_LOW_WARNING_MESSAGE_INDEX = 217,
#endif

    MAJOR_ALARM_ALL_FAN_FAILURE_INDEX = 218,
    MAJOR_ALARM_FAN_RECOVER_INDEX = 219,
    MINOR_ALARM_PARTITAL_FAN_FAIL_INDEX = 220,
    ALARM_INPUT_TYPE_INDEX = 221,
    MAJOR_ALARM_POWER_MODULE_SET_WRONG_INDEX = 222,
    MAJOR_ALARM_POWER_MODULE_SET_RECOVER_INDEX = 223,

#if (SYS_CPNT_BGP == TRUE)
    BGP_NEIGHBOR_CHANGE_MESSAGE_INDEX = 224,
    BGP_NEIGHBOR_MAX_PREFIX_OVERFLOW_NOTIFICATION_MESSAGE_INDEX = 224,
    BGP_ESTABLISHED_NOTIFICATION_MESSAGE_INDEX = 225,
    BGP_BACKWARD_TRANS_NOTIFICATION_MESSAGE_INDEX = 226,
#endif

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    CRAFT_PORT_LINK_UP_MESSAGE_INDEX = 228,     /* (UI32_T unit) */
    CRAFT_PORT_LINK_DOWN_MESSAGE_INDEX = 229,   /* (UI32_T unit) */
#endif

    XSTP_ROOT_BRIDGE_CHANGED_INDEX = 232,
#if (SYS_CPNT_VRRP == TRUE)
    VRRP_VM_OPER_STATE_CHANGE_INDEX = 233,
#endif
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    SFP_TX_POWER_ALARMWARN_CEASE_MESSAGE_INDEX = 234,
    SFP_RX_POWER_ALARMWARN_CEASE_MESSAGE_INDEX = 235,
    SFP_TEMP_ALARMWARN_CEASE_MESSAGE_INDEX = 236,
    SFP_VOLTAGE_ALARMWARN_CEASE_MESSAGE_INDEX = 237,
    SFP_CURRENT_ALARMWARN_CEASE_MESSAGE_INDEX = 238,
#endif

#if(SYS_CPNT_SYNCE == TRUE)
    SYNCE_SSM_RX_MESSAGE_INDEX    = 245,
    SYNCE_CLOCK_SRC_MESSAGE_INDEX = 246,
#endif
    ACTIVE_POWER_CHANGE_MESSAGE_INDEX = 247,

    USERAUTH_AUTHENTICATION_FAILURE_MESSAGE_INDEX = 248,
    USERAUTH_AUTHENTICATION_SUCCESS_MESSAGE_INDEX = 249,
    LOGIN_MESSAGE_INDEX = 250,
    LOGOUT_MESSAGE_INDEX = 251,

    XFER_FILE_COPY_MESSAGE_INDEX = 252,
    DHCPSNP_LOG_MESSAGE_INDEX = 253,

    USERAUTH_CREATE_USER_MESSAGE_INDEX = 254,
    USERAUTH_DELETE_USER_MESSAGE_INDEX = 255,
    USERAUTH_MODIFY_USER_PRIVILEGE_MESSAGE_INDEX = 256,

#if (SYS_CPNT_LLDP == TRUE)
    LLDP_REMOTE_TABLE_CHANGED_PER_PORT_MESSAGE_INDEX = 257,
#endif
    STA_TOPOLOGY_CHANGE_MESSAGE_INDEX_RECEIVE = 258,     /* void */

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
    CPU_GUARD_CONTROL_MESSAGE_INDEX = 259,
    CPU_GUARD_RELEASE_MESSAGE_INDEX = 260,
#endif
    IPSG_LOG_MESSAGE_INDEX = 264,
#if (SYS_CPNT_HTTP == TRUE)
    HTTP_LOG_MESSAGE_INDEX = 266,
#endif

#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP == TRUE)
    DYNAMIC_PRIVISION_CFG_OVERWRITE_MESSAGE_INDEX = 267,
    DYNAMIC_PRIVISION_START_PROVISION_MESSAGE_INDEX = 268,
    DYNAMIC_PRIVISION_INVALID_OPTION_MESSAGE_INDEX = 269,
    DYNAMIC_PRIVISION_SERVER_NAME_CANNOT_RESOLVED_MESSAGE_INDEX = 270,
    DYNAMIC_PRIVISION_DOWNLOAD_CFG_ERROR_MESSAGE_INDEX = 271,
#endif
};

#define  SYSLOG_MGR_Remote_Server_Config_T  SYSLOG_OM_Remote_Server_Config_T

#if (SYS_CPNT_SYSLOG_BACKUP == TRUE)
#define SYSLOG_MGR_MAX_NUM_OF_LOGFILE 2
#else
#define SYSLOG_MGR_MAX_NUM_OF_LOGFILE 1
#endif /* SYS_CPNT_SYSLOG_BACKUP */

typedef struct
{
    SYSLOG_OM_LogfileHeader_T   header;
    SYSLOG_OM_Record_T          entry[1];
} SYSLOG_MGR_PrepareCommon_T, * SYSLOG_MGR_PrepareCommonPtr_T;

typedef struct
{
    UI32_T                     session_type;
    char                       user_name[SYS_ADPT_MAX_USER_NAME_LEN+1];
    L_INET_AddrIp_T            user_ip;
    UI8_T                      user_mac[SYS_ADPT_MAC_ADDR_LEN];
} SYSLOG_MGR_UserInfo_T;

typedef struct SYSLOG_MGR_Prepare_S
{
    SYSLOG_OM_LogfileHeader_T   header;
    SYSLOG_OM_Record_T          entry[SYSLOG_ADPT_MAX_CAPABILITY_OF_PREPARE_DB];
} SYSLOG_MGR_Prepare_T;

#if (SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE)
typedef struct
{
    SYSLOG_OM_LogfileHeader_T   header;
    SYSLOG_OM_Record_T          entry[SYSLOG_ADPT_MAX_CAPABILITY_OF_PREPARE_LOGIN_OUT_DB];
} SYSLOG_MGR_PrepareLoginOut_T;
#endif /* (SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE) */

typedef struct SYSLOG_MGR_UcDatabase_S
{
    UI32_T  count;
    SYSLOG_OM_Record_T          entry[SYSLOG_ADPT_MAX_CAPABILITY_OF_TOTAL_UC_DB];
} SYSLOG_MGR_UcDatabase_T;

typedef struct SYSLOG_MGR_Rtc_S
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} SYSLOG_MGR_Rtc_T;

typedef struct
{
    UI32_T                          entry_index;
    BOOL_T                          valid;
} SYSLOG_MGR_RecordIndex_T;

typedef struct SYSLOG_MGR_Record_S
{
    UI32_T                          entry_index; /* Index of shown logs */
    SYSLOG_MGR_RecordIndex_T        index_ar[SYSLOG_OM_MAX_NBR_OF_PROFILE];
    SYSLOG_OM_RecordOwnerInfo_T   owner_info;
    SYSLOG_MGR_Rtc_T                rtc_time;      /* year/month/day/hour/minute/second
                                                    * If the device no support RTC,
                                                    * the RTC time(day/hour/minute/second).
                                                    */
    UI8_T   message[SYSLOG_ADPT_MESSAGE_LENGTH];

    /* For internal use to fetch log entry from core layer
     */
    UI32_T  internal_log_index;     /* Current index of logs (UC flash + log file) */
    UI32_T  internal_max_log_index; /* Max. index of logs (UC flash + log file) */
} SYSLOG_MGR_Record_T;

typedef struct
{
    union SYSLOG_MGR_IpcMsg_Type_U
    {
        UI32_T cmd;
        BOOL_T ret_bool;
        UI32_T ret_ui32;
    } type;

    union
    {
        struct SYSLOG_MGR_IPCMSG_SYSLOG_ENTRY_DATA_S
        {
            SYSLOG_OM_Record_T syslog_entry;
        } syslog_entry_data;

        struct SYSLOG_MGR_IPCMSG_OM_Data_S
        {
            UI32_T  syslog_status;      /* Enable/Disable */
            UI32_T  uc_log_level;       /* which level need to log to uc */
            UI32_T  flash_log_level;    /* which level nned to log to flash */
        }syslog_om_data;

        struct  SYSLOG_MGR_IPCMSG_Remote_Config_S
        {
            UI32_T  status;             /* Enable/Disable */
            UI32_T  server_index;
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE)
            UI32_T  facility;           /* which type need to log to server */
            UI32_T  level;              /* which level need to log to server */
#endif
            SYSLOG_MGR_Remote_Server_Config_T server_config;
        } syslog_remote_config;

        struct SYSLOG_MGR_IPCMSG_Record_S
        {
            UI32_T                          entry_index;
            SYSLOG_OM_RecordOwnerInfo_T     owner_info;
            SYSLOG_MGR_Rtc_T                rtc_time;
        }syslog_mgr_record;

/* Add a padding to make sure each string aragument have a null terminaled character.
 */
#define PAD 1
        struct SYSLOG_MGR_IPCMSG_Format_Msg_S
        {
            SYSLOG_OM_RecordOwnerInfo_T owner_info;
            UI32_T   message_index;
            UI8_T    arg_0[SYSLOG_ADPT_MESSAGE_LENGTH + PAD];
            UI8_T    arg_1[SYSLOG_ADPT_MESSAGE_LENGTH + PAD];
            UI8_T    arg_2[SYSLOG_ADPT_MESSAGE_LENGTH + PAD];
        }syslog_format_msg;
#undef PAD

        struct SYSLOG_MGR_IPCMSG_MGR_Record_S
        {
            SYSLOG_MGR_Record_T mgr_record;
        }syslog_mgr_record_s;

        SYSLOG_Host_T syslog_mgr_host;

    } data; /* contains the supplemntal data for the corresponding cmd */
} SYSLOG_MGR_IPCMsg_T;

/* definitions of command which will be used in ipc message
 */
enum
{
    SYSLOG_MGR_IPC_ADD_ENTRY,
    SYSLOG_MGR_IPC_ADD_FORMAT_MSG_ENTRY,
    SYSLOG_MGR_IPC_ADD_SERVER_IPADDR,
    SYSLOG_MGR_IPC_CLEAR_ALL_FLASH_ENTRIES,
    SYSLOG_MGR_IPC_CLEAR_ALL_RAM_ENTRIES,
    SYSLOG_MGR_IPC_GET_NEXT_UC_FLASH_ENTRY,
    SYSLOG_MGR_IPC_GET_NEXT_UC_NORMAL_ENTRIES,
    SYSLOG_MGR_IPC_GET_RUNNING_FACILITY_TYPE,
    SYSLOG_MGR_IPC_GET_RUNNING_FLASH_LOG_LEVEL,
    SYSLOG_MGR_IPC_GET_RUNNING_REMOTE_LOG_LEVEL,
    SYSLOG_MGR_IPC_GET_RUNNING_REMOTE_LOG_STATUS,
    SYSLOG_MGR_IPC_GET_RUNNING_SYSLOG_STATUS,
    SYSLOG_MGR_IPC_GET_RUNNING_UC_LOG_LEVEL,
    SYSLOG_MGR_IPC_SET_FLASH_LOG_LEVEL,
    SYSLOG_MGR_IPC_SET_REMOTE_LOG_STATUS,
    SYSLOG_MGR_IPC_SET_SYSLOG_STATUS,
    SYSLOG_MGR_IPC_SET_UC_LOG_LEVEL,
    SYSLOG_MGR_IPC_SNMP_GETNEXT_UC_FLASH_ENTRY,
    SYSLOG_MGR_IPC_SNMP_GETNEXT_UC_NORMAL_ENTRY,
    SYSLOG_MGR_IPC_SNMP_GET_UC_FLASH_ENTRY,
    SYSLOG_MGR_IPC_SNMP_GET_UC_NORMAL_ENTRY,
    SYSLOG_MGR_IPC_NOTIFYPROVISIONCOMPLETE,
    SYSLOG_MGR_IPC_GETSYSLOGSTATUS,
    SYSLOG_MGR_IPC_GETUCLOGLEVEL,
    SYSLOG_MGR_IPC_GETREMOTELOGSTATUS,
    SYSLOG_MGR_IPC_GETFLASHLOGLEVEL,
    SYSLOG_MGR_FOLLOWING_NON_RESP_CMD,
    SYSLOG_MGR_IPC_NOTIFYSTATPLGCHANGED,
    SYSLOG_MGR_IPC_NOTIFYSTATPLGSTABLED,
    SYSLOG_MGR_IPC_DELETE_ALL_SERVER,
    SYSLOG_MGR_IPC_CREATE_SERVER_IPADDR,
    SYSLOG_MGR_IPC_SET_SERVER_PORT,
    SYSLOG_MGR_IPC_GET_SERVER_PORT,
    SYSLOG_MGR_IPC_DELETE_SERVER_IPADDR,
    SYSLOG_MGR_IPC_GETSERVERCONFIG,
    SYSLOG_MGR_IPC_GET_NEXT_SERVER_CONFIG,
    SYSLOG_MGR_IPC_GETSERVERCONFIGINDEXBYIPADDR,
    SYSLOG_MGR_IPC_GET_RUNNING_SERVER_CONFIG,
     /*fuzhimin,20090414*/
#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
    SYSLOG_MGR_IPC_SET_SERVER_FACILITY,
    SYSLOG_MGR_IPC_SET_SERVER_LEVEL,
#else
    SYSLOG_MGR_IPC_SET_FACILITY,
    SYSLOG_MGR_IPC_GET_FACILITY,
    SYSLOG_MGR_IPC_SET_LEVEL,
    SYSLOG_MGR_IPC_GET_LEVEL,
#endif
     /*fuzhimin,20090414,end*/
    SYSLOG_MGR_IPC_ADD_ENTRY_SYNC,
};

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: SYSLOG_MGR_GetOperationMode()
 * PURPOSE:
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:
 * NOTES:
 *
 */
SYS_TYPE_Stacking_Mode_T SYSLOG_MGR_GetOperationMode(void);

/* FUNCTION NAME: SYSLOG_MGR_EnterMasterMode()
 * PURPOSE:
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_EnterMasterMode(void);


/* FUNCTION NAME: SYSLOG_MGR_SetTransitionMode()
 * PURPOSE:
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_SetTransitionMode(void);


/* FUNCTION NAME: SYSLOG_MGR_EntertTransitionMode()
 * PURPOSE:
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_EnterTransitionMode(void);


/* FUNCTION NAME: SYSLOG_MGR_EntertSlaveMode()
 * PURPOSE:
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_EnterSlaveMode(void);


/* FUNCTION NAME: SYSLOG_MGR_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the system log module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. Create semphore for syslog module using.
 *          2. Check syslog file exist or not in file system,
 *             If EXIST, nothing, otherwise create it.
 *          3. Set log_up_time = 0.
 *          4. Initialize OM database of the system log module.
 *          5. Call by SYSLOG_TASK_Initiate_System_Resources() only.
 *
 */
BOOL_T SYSLOG_MGR_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SYSLOG_MGR_Create_InterCSC_Relation(void);

/* FUNCTION NAME: SYSLOG_MGR_LoadDefaultOM
 * PURPOSE: This function is used to load the default OM value
 *          to the system log module when re-stacking.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. Initialize OM database of the system log module.
 *          2. Set log_up_time = 0.
 *          3. Call by SYSLOG_TAK_EnterTransitionMode() only.
 */
BOOL_T SYSLOG_MGR_LoadDefaultOM(void);


/* FUNCTION NAME: SYSLOG_MGR_GetSyslogStatus
 * PURPOSE: This function is used to get the system log status.
 * INPUT:   *syslog_status -- output buffer of system log status value.
 * OUTPUT:  *syslog_status -- system log status value.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_GetSyslogStatus(UI32_T *syslog_status);


/* FUNCTION NAME: SYSLOG_MGR_SetSyslogStatus
 * PURPOSE: This function is used to set the system log status.
 * INPUT:   syslog_status -- setting value of system log status.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_SetSyslogStatus(UI32_T syslog_status);


#if AARON
/* FUNCTION NAME: SYSLOG_MGR_EnableSyslog
 * PURPOSE: This function is used to enable system log status.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_EnableSyslog(void);


/* FUNCTION NAME: SYSLOG_MGR_DisableSyslog
 * PURPOSE: This function is used to disable system log status.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_DisableSyslog(void);
#endif


/* FUNCTION NAME: SYSLOG_MGR_GetUcLogLevel
 * PURPOSE: This function is used to get the un-cleared memory log level.
 * INPUT:   *uc_log_level -- output buffer of un-cleared memory log level value.
 * OUTPUT:  *uc_log_level -- un-cleared memory log level value.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. un-cleared memory log level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
BOOL_T SYSLOG_MGR_GetUcLogLevel(UI32_T *uc_log_level);


/* FUNCTION NAME: SYSLOG_MGR_SetUcLogLevel
 * PURPOSE: This function is used to set the un-cleared memory log level.
 * INPUT:   uc_log_level -- setting value of un-cleared memory log level.
 * RETUEN:  SYSLOG_RETURN_OK                                --  OK, Successful, Without any Error
 *          SYSLOG_NORMAL_LEVEL_SMALLER_THAN_FLASH_LEVEL    --  Normal level is smaller than flash level
 *          SYSLOG_FLASH_LEVEL_BIGGER_THAN_NORMAL_LEVEL     --  Flash level is bigger than normal level
 *          SYSLOG_LEVEL_VLAUE_INVALID                      --  Syslog level is invalid
 *
 * NOTES:   1. un-cleared memory log level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *          3. un-cleared memory log level must be lower priority than
 *             un-cleared memory flash level.
 *
 */
UI32_T SYSLOG_MGR_SetUcLogLevel(UI32_T uc_log_level);


/* FUNCTION NAME: SYSLOG_MGR_GetFlashLogLevel
 * PURPOSE: This function is used to get the flash log level.
 * INPUT:   *flash_log_level -- output buffer of flash log level value.
 * OUTPUT:  *flash_log_level -- flash log level value.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. un-cleared memory log level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
BOOL_T SYSLOG_MGR_GetFlashLogLevel(UI32_T *flash_log_level);


/* FUNCTION NAME: SYSLOG_MGR_SetFlashLogLevel
 * PURPOSE: This function is used to set the flash log level.
 * INPUT:   flash_log_level -- setting value of flash log level.
 * OUTPUT:  None.
 * RETUEN:  SYSLOG_RETURN_OK                                --  OK, Successful, Without any Error
 *          SYSLOG_NORMAL_LEVEL_SMALLER_THAN_FLASH_LEVEL    --  Normal level is smaller than flash level
 *          SYSLOG_FLASH_LEVEL_BIGGER_THAN_NORMAL_LEVEL     --  Flash level is bigger than normal level
 *          SYSLOG_LEVEL_VLAUE_INVALID                      --  Syslog level is invalid
 *
 * NOTES:   1. un-cleared memory log level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *          3. un-cleared memory flash level must be higher priority than
 *             un-cleared memory log level.
 *
 */
UI32_T SYSLOG_MGR_SetFlashLogLevel(UI32_T flash_log_level);


/* FUNCTION NAME: SYSLOG_MGR_AddFormatMsgEntry
 * PURPOSE: Add a log message to system log module using format message.
 * INPUT:   *syslog_entry   -- add this syslog entry
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T
SYSLOG_MGR_AddFormatMsgEntry(
    SYSLOG_OM_RecordOwnerInfo_T *owner_info,
    UI32_T   message_index,
    void     *arg_0,
    void     *arg_1,
    void     *arg_2
);

/* FUNCTION NAME: SYSLOG_MGR_AddEntrySync
 * PURPOSE: Add a log message to system log module synchrously.
 * INPUT:   *syslog_entry   -- add this syslog entry
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *          If the entry will be written to flash, it
 *          will be done after calling this function.
 */
BOOL_T SYSLOG_MGR_AddEntrySync(SYSLOG_OM_Record_T *syslog_entry);

/* FUNCTION NAME: SYSLOG_MGR_AddEntry
 * PURPOSE: Add a log message to system log module.
 * INPUT:   *syslog_entry   --
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_AddEntry(SYSLOG_OM_Record_T *syslog_entry);


/* FUNCTION NAME: SYSLOG_MGR_ClearAllRamEntries
 * PURPOSE: Clear all log message from system log module in UC RAM memory.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. This function only clear the UC RAM database, but not clear
 *          the UC Flash and file system syslog file.
 */
BOOL_T
SYSLOG_MGR_ClearAllRamEntries(
    void);


/* FUNCTION NAME: SYSLOG_MGR_ClearAllFlashEntries
 * PURPOSE: Clear all log message from system log module in flash and UC Flash.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. This function clear the file system syslog file.
 *
 */
BOOL_T SYSLOG_MGR_ClearAllFlashEntries(void);


/* FUNCTION NAME: SYSLOG_MGR_LogUcFlashDbToLogFile
 * PURPOSE: Log the un-cleared flash database to file system if exist any entry.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   This function will be active in following 2 events.
 *          -- uc flash database is full.
 *          -- uc flash database is not stroed to file system excess 12 hours.
 *
 */
BOOL_T SYSLOG_MGR_LogUcFlashDbToLogFile(void);


/* FUNCTION NAME: SYSLOG_MGR_NotifyProvisionComplete
 * PURPOSE: STKCTRL notify SYSLOG when STKCTRL know the provision complete..
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. This function only use by STKCTRL.
 *
 */
void SYSLOG_MGR_NotifyProvisionComplete(void);


#if 0 /* replace by new APIs, 2002/1/24 */
/* FUNCTION NAME: SYSLOG_MGR_GetNextUcNormalEntries
 * PURPOSE: Get next log messages from un-cleared memory normal log DB.
 * INPUT:   *record->entry_index    -- entry index.
 * OUTPUT:  *record                 -- next entry.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure or no next entry.
 * NOTES:   1. Use *record->entry_index = (UI32_T) (-1) to get first entry.
 *          2. Use *record->entry_index = i to get index(i+1) entry.
 *          3. CLI will call this function.
 */
BOOL_T SYSLOG_MGR_GetNextUcNormalEntries(SYSLOG_MGR_Record_T *record);


/* FUNCTION NAME: SYSLOG_MGR_GetNextUcFlashEntry
 * PURPOSE: Get next log message from un-cleared memory flash log DB.
 * INPUT:   *record->entry_index    -- entry index.
 * OUTPUT:  *record                 -- next entry.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure or no next entry.
 * NOTES:   1. Use *record->entry_index = (UI32_T) (-1) to get first entry.
 *          2. Use *record->entry_index = i to get index(i+1) entry.
 *          3. CLI will call this function.
 */
BOOL_T SYSLOG_MGR_GetNextUcFlashEntry(SYSLOG_MGR_Record_T *record);
#endif


/* FUNCTION NAME: SYSLOG_MGR_GetNextUcNormalEntries
 * PURPOSE: Get next log message from un-cleared memory DB.
 * INPUT:   *record->entry_index    -- entry index.
 * OUTPUT:  *record                 -- next entry.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure or no next entry.
 * NOTES:   1. Use *record->entry_index = (UI32_T) (-1) to get first entry.
 *          2. Use *record->entry_index = i to get index(i+1) entry.
 *          3. CLI will call this function.
 */
BOOL_T SYSLOG_MGR_GetNextUcNormalEntries(SYSLOG_MGR_Record_T *mgr_record);


/* FUNCTION NAME: SYSLOG_MGR_GetNextUcFlashEntry
 * PURPOSE: Get next log message from logfile in file system.
 * INPUT:   *record->entry_index    -- entry index.
 * OUTPUT:  *record                 -- next entry.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure or no next entry.
 * NOTES:   1. Use *record->entry_index = (UI32_T) (-1) to get first entry.
 *          2. Use *record->entry_index = i to get index(i+1) entry.
 *          3. CLI will call this function.
 */
BOOL_T SYSLOG_MGR_GetNextUcFlashEntry(SYSLOG_MGR_Record_T *mgr_record);

/* FUNCTION NAME: SYSLOG_MGR_LogColdStartEntry
 * PURPOSE: Log a cold start entry to uc-memory, and mark a RTC/SysUpTime information.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   SYSMGMT will call this function to notify SYSLOG cold start happen.
 *
 */
BOOL_T SYSLOG_MGR_LogColdStartEntry(void);


/* FUNCTION NAME: SYSLOG_MGR_LogWarmStartEntry
 * PURPOSE: Log a warm entry to uc-memory, and mark a RTC/SysUpTime information.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   SYSMGMT will call this function to notify SYSLOG warm start happen.
 *
 */
BOOL_T SYSLOG_MGR_LogWarmStartEntry(void);


/* FUNCTION NAME: SYSLOG_MGR_GetRunningSyslogStatus
 * PURPOSE: This function is used to get the non-default system log status.
 * INPUT:   *syslog_status -- syslog status output buffer.
 * OUTPUT:  *syslog_status -- syslog status.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. Default is SYSLOG_STATUS_DISABLE.
 *
 */
UI32_T SYSLOG_MGR_GetRunningSyslogStatus(UI32_T *syslog_status);


/* FUNCTION NAME: SYSLOG_MGR_GetRunningUcLogLevel
 * PURPOSE: This function is used to get the non-default un-cleared memory log level.
 * INPUT:   *uc_log_level -- output buffer of un-cleared memory log level value.
 * OUTPUT:  *uc_log_level -- un-cleared memory log level value.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. Default is SYSLOG_LEVEL_DEBUG.
 *
 */
UI32_T SYSLOG_MGR_GetRunningUcLogLevel(UI32_T *uc_log_level);


/* FUNCTION NAME: SYSLOG_MGR_GetRunningFlashLogLevel
 * PURPOSE: This function is used to get the non-default flash log level.
 * INPUT:   *flash_log_level -- output buffer of flash log level value.
 * OUTPUT:  *flash_log_level -- flash log level value.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. Default is SYSLOG_LEVEL_ERR.
 *
 */
UI32_T SYSLOG_MGR_GetRunningFlashLogLevel(UI32_T *flash_log_level);


/* FUNCTION NAME: SYSLOG_MGR_TimerExpiry
 * PURPOSE: This routine will be called to check expiry or not.
 *          If Expiry, log entries from uc flash db to file system.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function be called per timer interval (30 sec).
 *
 */
void SYSLOG_MGR_TimerExpiry(void);


/* FUNCTION NAME: SYSLOG_MGR_RegisterUcFlashDbFull_CallBack
 * PURPOSE: Register the call-back function, when uc flash db is full
 *          the register function will be called
 * INPUT:   fun -- call back function pointer.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   SYSLOG_TASK will register this function.
 *
 */
void SYSLOG_MGR_RegisterUcFlashDbFull_CallBack(void (*fun)(void));


/* Tempernary, 2001/10/18 by Aaron */
void SYSLOG_MGR_SyslogEngineerDebugMenu (void);


SYSLOG_MGR_Prepare_T *SYSLOG_MGR_GetPrepareDbPointer();

SYSLOG_MGR_UcDatabase_T *SYSLOG_MGR_GetUcDatabasePointer();
#if (SYS_CPNT_SYSLOG_BACKUP == TRUE)
BOOL_T SYSLOG_MGR_RecoveryLogfileInFileSystem(void);
#endif

#if (SYS_CPNT_REMOTELOG == TRUE)
/* FUNCTION NAME: SYSLOG_MGR_LoadDefaultOM_Remote
 * PURPOSE: This function is used to load the default OM value
 *          to the remote log module when re-stacking.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. Initialize OM database of the remote log module.
 *          2. Set log_up_time = 0.
 *          3. Call by REMOTELOG_TAK_EnterTransitionMode() only.
 */
BOOL_T SYSLOG_MGR_LoadDefaultOM_Remote(void);

/* FUNCTION NAME: SYSLOG_MGR_CreateRemoteLogServer
 * PURPOSE: This function is used to add the server ip address.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 */
UI32_T
SYSLOG_MGR_CreateRemoteLogServer(
    L_INET_AddrIp_T *ip_address
);

/* FUNCTION NAME: SYSLOG_MGR_DeleteRemoteLogServer
 * PURPOSE: This function is used to delete the server ip address.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 */
UI32_T
SYSLOG_MGR_DeleteRemoteLogServer(
    L_INET_AddrIp_T *ip_address
);

/* FUNCTION NAME: SYSLOG_MGR_SetRemoteLogServerPort
 * PURPOSE: This function is used to add the server ip address.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *
 */
UI32_T
SYSLOG_MGR_SetRemoteLogServerPort(
    L_INET_AddrIp_T * ip_address,
    UI32_T port
);

/* FUNCTION NAME: SYSLOG_MGR_GetRemoteLogServerPort
 * PURPOSE: This function is used to get the port number of the input server
 *          ip address .
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  *port_p    -- port number of the input server ip address
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *
 */
UI32_T
SYSLOG_MGR_GetRemoteLogServerPort(
    L_INET_AddrIp_T *ip_address,
    UI32_T *port_p
);

/* FUNCTION NAME: SYSLOG_MGR_GetServerIPAddr
 * PURPOSE: This function is used to get the server ip address.
 * INPUT:   index       -- index of server ip address.
 *          *ip_address -- output buffer of server ip address.
 * OUTPUT:  *ip_address -- server ip address.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *          2. get all server ip address one time
 *          3. index value from 0 to SYS_ADPT_REMOTELOG_MAX_SERVER_NBR-1
 *
 */
UI32_T
SYSLOG_MGR_GetServerIPAddr(
    UI8_T index,
    L_INET_AddrIp_T *ip_address
);

/* FUNCTION NAME: SYSLOG_MGR_GetNextRunningServerIPAddr
 * PURPOSE: This function is used to get the next running server ip address.
 * INPUT:   *ip_address -- output buffer of server ip address.
 * OUTPUT:  *ip_address -- server ip address.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   index value from 0 to SYS_ADPT_REMOTELOG_MAX_SERVER_NBR-1
 *
 */
UI32_T
SYSLOG_MGR_GetNextRunningServerIPAddr(
    L_INET_AddrIp_T *ip_address
);

/* FUNCTION NAME: SYSLOG_MGR_DeleteAllRemoteLogServer
 * PURPOSE: This function is used to delete all server ip address.
 * INPUT:
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *
 */
UI32_T
SYSLOG_MGR_DeleteAllRemoteLogServer(
);

/* FUNCTION NAME: SYSLOG_MGR_GetRunningFacilityType
 * PURPOSE: This function is used to get running facility type.
 * INPUT:   *facility -- output buffer of facility type.
 * OUTPUT:  *facility -- facility type
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. facility type is defined as following:
 *             REMOTELOG_FACILITY_LOCAL0      = 16;
 *             REMOTELOG_FACILITY_LOCAL1      = 17;
 *             REMOTELOG_FACILITY_LOCAL2      = 18;
 *             REMOTELOG_FACILITY_LOCAL3      = 19;
 *             REMOTELOG_FACILITY_LOCAL4      = 20;
 *             REMOTELOG_FACILITY_LOCAL5      = 21;
 *             REMOTELOG_FACILITY_LOCAL6      = 22;
 *             REMOTELOG_FACILITY_LOCAL7      = 23;
 *          2. default is REMOTELOG_FACILITY_LOCAL7
 *
 */
UI32_T
SYSLOG_MGR_GetRunningFacilityType(
    UI32_T *facility
);

/* FUNCTION NAME: SYSLOG_MGR_SetRemoteLogStatus
 * PURPOSE: This function is used to enable/disable REMOTELOG.
 * INPUT:   status -- remotelog status.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. status is defined as following:
 *             SYSLOG_STATUS_ENABLE
 *             REMOTELOG_STATUS_DISABLE
 *
 */
UI32_T
SYSLOG_MGR_SetRemoteLogStatus(
    UI32_T status
);

/* FUNCTION NAME: SYSLOG_MGR_GetRemoteLogStatus
 * PURPOSE: This function is used to get remotelog status.
 * INPUT:   *status -- output buffer of remotelog status.
 * OUTPUT:  *status -- remotelog status.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. status is defined as following:
 *             SYSLOG_STATUS_ENABLE
 *             REMOTELOG_STATUS_DISABLE
 *
 */
UI32_T
SYSLOG_MGR_GetRemoteLogStatus(
    UI32_T *status
);

/* FUNCTION NAME: SYSLOG_MGR_GetRunningRemoteLogStatus
 * PURPOSE: This function is used to get running remotelog status.
 * INPUT:   *status -- output buffer of remotelog status.
 * OUTPUT:  *status -- remotelog status.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. status is defined as following:
 *             SYSLOG_STATUS_ENABLE
 *             REMOTELOG_STATUS_DISABLE
 *          2. default status is SYSLOG_STATUS_ENABLE
 *
 */
UI32_T
SYSLOG_MGR_GetRunningRemoteLogStatus(
    UI32_T *status
);

/* FUNCTION NAME: SYSLOG_MGR_GetRunningRemotelogLevel
 * PURPOSE: This function is used to get running remotelog level.
 * INPUT:   *level -- output buffer of remotelog level.
 * OUTPUT:  *level -- remotelog level.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. remotelog level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
UI32_T SYSLOG_MGR_GetRunningRemotelogLevel(UI32_T *level);

/* FUNCTION NAME: SYSLOG_MGR_HandleTrapQueue
 * PURPOSE: This function is used to handle remotelog trap queue.
 * INPUT:   None
 * OUTPUT:  None.
 * RETUEN:  None
 * NOTES:   None.
 *
 */
void SYSLOG_MGR_HandleTrapQueue(void);

/* FUNCTION NAME: SYSLOG_MGR_SetTaskId
 * PURPOSE: This function is used to keep syslog task id for remotelog send event use.
 * INPUT:   task_id -- task id
 * OUTPUT:  None.
 * RETUEN:  None
 * NOTES:   None.
 *
 */
void SYSLOG_MGR_SetTaskId(UI32_T task_id);

/* FUNCTION NAME: SYSLOG_MGR_GetStaState
 * PURPOSE: This function is used to get sta state.
 * INPUT:   *sta_state -- sta state
 * OUTPUT:  None.
 * RETUEN:  None
 * NOTES:   None.
 *
 */
void SYSLOG_MGR_GetStaState(UI32_T *sta_state);

/* FUNCTION NAME: SYSLOG_MGR_SetStaState
 * PURPOSE: This function is used to set sta state.
 * INPUT:   *sta_state -- sta state
 * OUTPUT:  None.
 * RETUEN:  None
 * NOTES:   This API is for syslog task use only
 *
 */
void SYSLOG_MGR_SetStaState(UI32_T sta_state);

/* FUNCTION NAME: SYSLOG_MGR_IsQueueEmpty
 * PURPOSE: This function is used to check if queue is empty.
 * INPUT:   None
 * OUTPUT:  None.
 * RETUEN:  TRUE -- Queue is empty
 *          FALSE -- Queue is not empty
 * NOTES:   None.
 *
 */
BOOL_T SYSLOG_MGR_IsQueueEmpty(void);

/* FUNCTION NAME: SYSLOG_MGR_GetNextRemoteLogServer
 * PURPOSE: This function is used to get the server ip address.
 * INPUT:   *ip_address -- output buffer of server ip address.
 *          index       -- index of server ip address.
 * OUTPUT:  *ip_address -- server ip address.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *          2. get all server ip address one time
 *          3. index value from 0 to SYS_ADPT_REMOTELOG_MAX_SERVER_NBR-1
 *
 */
UI32_T
SYSLOG_MGR_GetNextRemoteLogServer(
    SYSLOG_MGR_Remote_Server_Config_T *server_config
);

/* FUNCTION NAME: SYSLOG_MGR_GetRemoteLogServer
 * PURPOSE: This function is used to get the server ip address.
 * INPUT:   *ip_address -- output buffer of server ip address.
 *          index       -- index of server ip address.
 * OUTPUT:  *ip_address -- server ip address.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *          2. get all server ip address one time
 *          3. index value from 0 to SYS_ADPT_REMOTELOG_MAX_SERVER_NBR-1
 *
 */
UI32_T
SYSLOG_MGR_GetRemoteLogServer(
    SYSLOG_MGR_Remote_Server_Config_T *server_config
);

/* FUNCTION NAME: SYSLOG_MGR_GetRunningRemoteLogServer
 * PURPOSE: This function is used to get the running server ip address.
 * INPUT:   *ip_address -- output buffer of server ip address.
 *          index       -- index of server ip address.
 * OUTPUT:  *ip_address -- server ip address.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   index value from 0 to SYS_ADPT_REMOTELOG_MAX_SERVER_NBR-1
 *
 */
UI32_T
SYSLOG_MGR_GetRunningRemoteLogServer(
    SYSLOG_MGR_Remote_Server_Config_T *server_config,
    UI32_T index
);

#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
/* FUNCTION NAME: SYSLOG_MGR_AddNewServerIPAddr
 * PURPOSE: This function is used to add the server ip address.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *
 */
UI32_T
SYSLOG_MGR_AddNewServerIPAddr(
    L_INET_AddrIp_T * ip_address
);

/* FUNCTION NAME: SYSLOG_MGR_AddNewServerIPAddrFacility
 * PURPOSE: This function is used to add the server ip address.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *
 */
UI32_T
SYSLOG_MGR_AddNewServerIPAddrFacility(
    L_INET_AddrIp_T * ip_address,
    UI32_T facility
);

/* FUNCTION NAME: SYSLOG_MGR_AddNewServerIPAddrLevel
 * PURPOSE: This function is used to add the server ip address.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *
 */
UI32_T
SYSLOG_MGR_AddNewServerIPAddrLevel(
    L_INET_AddrIp_T * ip_address,
    UI32_T level
);

/* FUNCTION NAME: SYSLOG_MGR_AddNewServerIPAddrFacilityLevel
 * PURPOSE: This function is used to add the server ip address.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *
 */
UI32_T
SYSLOG_MGR_AddNewServerIPAddrFacilityLevel(
    L_INET_AddrIp_T * ip_address,
    UI32_T facility,
    UI32_T level
);

/* FUNCTION NAME: SYSLOG_MGR_SetRemoteLogServerFacility
 * PURPOSE: This function is used to set the facility of input host.
 * INPUT:   ip_address -- server ip address.
            facility   -- facility level
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *
 */
UI32_T
SYSLOG_MGR_SetRemoteLogServerFacility(
    L_INET_AddrIp_T * ip_address,
    UI32_T facility
);

/* FUNCTION NAME: SYSLOG_MGR_SetRemoteLogServerLevel
 * PURPOSE: This function is used to add the server ip address.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *
 */
UI32_T
SYSLOG_MGR_SetRemoteLogServerLevel(
    L_INET_AddrIp_T * ip_address,
    UI32_T level
);

#else
/* FUNCTION NAME: SYSLOG_MGR_SetRemoteLogFacility
 * PURPOSE: This function is used to set facility type.
 * INPUT:   facility -- facility type.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. facility type is defined as following:
 *             SYSLOG_REMOTE_FACILITY_LOCAL0      = 16;
 *             SYSLOG_REMOTE_FACILITY_LOCAL1      = 17;
 *             SYSLOG_REMOTE_FACILITY_LOCAL2      = 18;
 *             SYSLOG_REMOTE_FACILITY_LOCAL3      = 19;
 *             SYSLOG_REMOTE_FACILITY_LOCAL4      = 20;
 *             SYSLOG_REMOTE_FACILITY_LOCAL5      = 21;
 *             SYSLOG_REMOTE_FACILITY_LOCAL6      = 22;
 *             SYSLOG_REMOTE_FACILITY_LOCAL7      = 23;
 *          2. default is SYSLOG_REMOTE_FACILITY_LOCAL7
 *
 */
UI32_T
SYSLOG_MGR_SetRemoteLogFacility(
    UI32_T facility
);

/* FUNCTION NAME: SYSLOG_MGR_GetRemoteLogFacility
 * PURPOSE: This function is used to get facility type.
 * INPUT:   *facility_p -- output buffer of facility type.
 * OUTPUT:  *facility_p -- facility type
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. facility type is defined as following:
 *             SYSLOG_REMOTE_FACILITY_LOCAL0      = 16;
 *             SYSLOG_REMOTE_FACILITY_LOCAL1      = 17;
 *             SYSLOG_REMOTE_FACILITY_LOCAL2      = 18;
 *             SYSLOG_REMOTE_FACILITY_LOCAL3      = 19;
 *             SYSLOG_REMOTE_FACILITY_LOCAL4      = 20;
 *             SYSLOG_REMOTE_FACILITY_LOCAL5      = 21;
 *             SYSLOG_REMOTE_FACILITY_LOCAL6      = 22;
 *             SYSLOG_REMOTE_FACILITY_LOCAL7      = 23;
 *          2. default is SYSLOG_REMOTE_FACILITY_LOCAL7
 *
 */
UI32_T
SYSLOG_MGR_GetRemoteLogFacility(
    UI32_T *facility_p
);

/* FUNCTION NAME: SYSLOG_MGR_SetRemoteLogLevel
 * PURPOSE: This function is used to add the server ip address.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *
 */
UI32_T
SYSLOG_MGR_SetRemoteLogLevel(
    UI32_T level
);

/* FUNCTION NAME: SYSLOG_MGR_GetRemoteLogLevel
 * PURPOSE: This function is used to get remotelog level.
 * INPUT:   *level_p -- output buffer of remotelog level.
 * OUTPUT:  *level_p -- remotelog level.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. remotelog level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
UI32_T
SYSLOG_MGR_GetRemoteLogLevel(
    UI32_T *level_p
);

#endif /*endif (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)*/
#endif //end (SYS_CPNT_REMOTELOG == TRUE)

/* FUNCTION NAME: SYSLOG_MGR_SnmpGetUcNormalEntry
 * PURPOSE: Get current log message from un-cleared memory DB.
 * INPUT:   *record->entry_index    -- entry index.
 * OUTPUT:  *record                 -- current entry.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure or no current entry.
 * NOTES:   1. Use *record->entry_index = 1 to get first entry.
 *          2. SNMP will call this function.
 */
BOOL_T SYSLOG_MGR_SnmpGetUcNormalEntry(SYSLOG_MGR_Record_T *mgr_record);

/* FUNCTION NAME: SYSLOG_MGR_SnmpGetNextUcNormalEntry
 * PURPOSE: Get next log message from un-cleared memory DB.
 * INPUT:   *record->entry_index    -- entry index.
 * OUTPUT:  *record                 -- next entry.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure or no next entry.
 * NOTES:   1. Use *record->entry_index = 1 to get first entry.
 *          2. SNMP will call this function.
 */
BOOL_T SYSLOG_MGR_SnmpGetNextUcNormalEntry(SYSLOG_MGR_Record_T *mgr_record);

/* ----------------------------------------------------------------------------
 *  ROUTINE NAME  - SYSLOG_MGR_NotifyStaTplgChanged
 * -----------------------------------------------------------------------------
 * Purpose: This procedure notify that network topology is changed due to the
 *          STA enabled.
 * Parameter:
 * Return: None.
 * Note: When STA enabled, all the ports will go through STA algorithm to
 *       determine its operation state. During this period, the trap management
 *       shall wait until STA becomes stable. Otherwise, the trap message
 *       will be lost if the port is not in forwarding state.
 * -----------------------------------------------------------------------------
 */
void SYSLOG_MGR_NotifyStaTplgChanged (void);

/* ----------------------------------------------------------------------------
 *  ROUTINE NAME  - SYSLOG_MGR_NotifyStaTplgStabled
 * -----------------------------------------------------------------------------
 * Purpose: This procedure notify that STA has been enabled, and at least one of the port enters
 *          forwarding state. The network topology shall be stabled after couple seconds.
 * Parameter:
 * Return: None.
 * Note: This notification only informs that at least one of STA port enters forwarding state.
 *       To make sure all the STA ports enters stable state, we shall wait for few more seconds
 *       before we can send trap messages.
 * -----------------------------------------------------------------------------
 */
void SYSLOG_MGR_NotifyStaTplgStabled (void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
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
 * ------------------------------------------------------------------------
 */
void SYSLOG_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void SYSLOG_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/* FUNCTION NAME: SYSLOG_MGR_AddRawMsgEntry
 * PURPOSE: Add a log message to system log module using raw message.
 * INPUT:   level_no   -- level number
 *          module_no  -- module number,defined in sys_module.h
 *          function_no -- function number,defined by CSC
 *          error_no   -- error number,defined by CSC
 *          message_index -- message index
 *          *arg_0     -- input argument 0
 *          *arg_1     -- input argument 1
 *          *arg_2     -- input argument 2
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T
SYSLOG_MGR_AddRawMsgEntry(
    UI8_T level_no,
    UI32_T module_no,
    UI8_T function_no,
    UI8_T error_no,
    UI32_T message_index,
    void *arg_0,
    void *arg_1,
    void *arg_2
);

UI32_T
SYSLOG_MGR_RifUp_CallBack(
    UI32_T ip_address,
    UI32_T ip_mask
);

UI32_T
SYSLOG_MGR_RifDown_CallBack(
    UI32_T ip_address,
    UI32_T ip_mask
);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for SYSLOG MGR.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYSLOG_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/* FUNCTION NAME: SYSLOG_MGR_SaveUcLogsToFlash
 * PURPOSE: Save UC log data on UC memory to Flash.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
void SYSLOG_MGR_SaveUcLogsToFlash();

/* MACRO FUNCTION DECLARATIONS
 */
#endif /* SYSLOG_MGR_H */
