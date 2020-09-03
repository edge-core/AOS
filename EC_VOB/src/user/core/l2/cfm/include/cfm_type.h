/*-----------------------------------------------------------------------------
 * Module Name: cfm_type.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the CFM types
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/15/2006 - Macauley, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */
#ifndef _CFM_TYPE_H
#define _CFM_TYPE_H

#include "sys_type.h"
#include "sys_module.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "sys_time.h"
#include "l_mm.h"
#include "leaf_ieee8021ag.h"
#if 0
 /*redefine just for reduce warning*/
#define strcmp(X, Y)  strcmp((char *)X, (char  *)Y)
#define strcpy(X, Y)  strcpy((char *)X, (char *) Y)
#define strchr(X, Y)  strchr((char *)X, Y)
#define strlen(X)  strlen((char *)X)
#define strncpy(X, Y, Z) strncpy((char *)X, (char *)Y, Z)
#endif
#if (SYS_CPNT_CFM == TRUE)
#define CFM_TYPE_LBM_TIMEOUT_TIME                5         /*sec*/
#define CFM_TYPE_MIN_LBM_COUNT                   1
#define CFM_TYPE_MAX_LBM_COUNT                   MAX_dot1agCfmMepTransmitLbmMessages
#define CFM_TYPE_MIN_LBM_PKTSIZE                 64
#define CFM_TYPE_MAX_LBM_PKTSIZE                 1518

#define CFM_TYPE_DEF_LBM_COUNT                   5
#define CFM_TYPE_DEF_LBM_PKTSIZE                 64
#define CFM_TYPE_DEF_LBM_TIMEOUT                 5

#define CFM_TYPE_MAX_PDU_PRIORITY                7
#define CFM_TYPE_MIN_PDU_PRIORITY                0
#define CFM_TYPE_DEF_PDU_PRIORITY                7

#define CFM_TYPE_MAX_LBM_PATTERN                 0xffff
#define CFM_TYPE_MIN_LBM_PATTERN                 0x0
#define CFM_TYPE_DEF_LBM_PATTERN                 0

#define CFM_TYPE_MIN_DMM_COUNT                   1
#define CFM_TYPE_MAX_DMM_COUNT                   5
#define CFM_TYPE_MIN_DMM_INTERVAL                1
#define CFM_TYPE_MAX_DMM_INTERVAL                5
#define CFM_TYPE_MIN_DMM_TIMEOUT                 1
#define CFM_TYPE_MAX_DMM_TIMEOUT                 5
/* total size of frame (including header)
 */
#define CFM_TYPE_MIN_DMM_PKTSIZE                 64
#define CFM_TYPE_MAX_DMM_PKTSIZE                 1518

#define CFM_TYPE_DEF_DMM_COUNT                   5
#define CFM_TYPE_DEF_DMM_INTERVAL                1
#define CFM_TYPE_DEF_DMM_TIMEOUT                 5
#define CFM_TYPE_DEF_DMM_PKTSIZE                 64

#define CFM_TYPE_MAID_NAME_LENGTH                48        /*maid now is defined by packet format, don't modify this unless packet content change*/
#define CFM_TYPE_CCM_ITU_FIELD_LENGTH            16        /*59-74, 16 bytes, define by ITU-T Y.1731*/


/* CFM Timer */
#define CFM_TYPE_TIMER_TICKS2SEC                 SYS_BLD_TICKS_PER_SECOND
#define CFM_TYPE_TIME_UNIT                       SYS_BLD_TICKS_PER_SECOND


#define CFM_TYPE_CFM_ETHER_TYPE                  0x8902

#define CFM_TYPE_LOG_FUN_CFM_TASK_CREATE_TASK    0
#define CFM_TYPE_LOG_ERR_CFM_TASK_CREATE_TASK    1

#define CFM_TYPE_MAX_CHASSIS_ID_LENGTH           33
#define CFM_TYPE_MAX_PORT_ID_LENGTH              33
#define CFM_TYPE_MAX_MAN_ADDRESS_LENGTH          33
#define CFM_TYPE_MAX_MAN_DOMAIN_LENGTH           40

/* this size is not including the (da + sa + ethertype + fcs)
 */
#define CFM_TYPE_MAX_FRAME_SIZE                  1500  /*define the frame size to send the packet*/
#define CFM_TYPE_MAX_FRAME_RECORD_SIZE           500   /*define the frame size to store the received packet*/
#define CFM_TYPE_MAX_ORGANIZATION_TLV_LENGTH     100   /*define the frame size to store the organization tlv*/
#define CFM_TYPE_MAX_UNKNOWN_TLV_SIZE            150

#define CFM_TYPE_EVENT_NONE                      0
#define CFM_TYPE_EVENT_TIMER                     (1 << 0)
#define CFM_TYPE_EVENT_CFMDURCVD                 (1 << 1)
#define CFM_TYPE_EVENT_ENTER_TRANSITION          (1 << 2)
#define CFM_TYPE_EVENT_ALL                       0x0F

/*defien remote mep related constant*/
#define CFM_TYPE_MAX_CROSSCHECK_START_DELAY     65535
#define CFM_TYPE_MIN_CROSSCHECK_START_DELAY     1

#define CFM_TYPE_MD_MAX_NAME_LENGTH             MAXSIZE_dot1agCfmMdName
#define CFM_TYPE_MD_MIN_NAME_LENGTH             MINSIZE_dot1agCfmMdName

#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
    /* since the minimum length of MD name is 1,
     * the maxmimum length should be 45 - 2 = 43
     */
    #define CFM_TYPE_MA_MAX_NAME_LENGTH             (MAXSIZE_dot1agCfmMaName-2)
#else
    #define CFM_TYPE_MA_MAX_NAME_LENGTH             (MAXSIZE_dot1agCfmMaName)
#endif

#define CFM_TYPE_MA_MIN_NAME_LENGTH             MINSIZE_dot1agCfmMaName

#define CFM_TYPE_MA_MAX_NAME_LENGTH_FOR_Y1731   13

/*define fng alarm, reset time in ticks*/
#define CFM_TYPE_FNG_MAX_ALARM_TIME            (MAX_dot1agCfmMepFngAlarmTime/100)      /*sec*/
#define CFM_TYPE_FNG_MIN_ALARM_TIME            3
#define CFM_TYPE_FNG_MAX_RESET_TIME            (MAX_dot1agCfmMepFngResetTime/100)      /*sec*/
#define CFM_TYPE_FNG_MIN_RESET_TIME            3


/*define achive hold time in minutes*/
#define CFM_TYPE_MAX_ARCHIVE_HOLD_TIME         65535
#define CFM_TYPE_MIN_ARCHIVE_HOLD_TIME         1

/*define ltm link trace hold time in minutes*/
#define CFM_TYPE_MAX_LINKTRACE_HOLD_TIME       65535
#define CFM_TYPE_MIN_LINKTRACE_HOLD_TIME       1
#define CFM_TYPE_MAX_LINKTRACE_SIZE            SYS_ADPT_CFM_MAX_NBR_OF_LTR
#define CFM_TYPE_MIN_LINKTRACE_SIZE            1


#define CFM_TYPE_DFT_SEQUENCE_NUM              1
#define CFM_TYPE_DFT_NEXT_TRANSAS_ID           1 /* 1st to be send, should be the same as CFM_TYPE_DFT_SEQUENCE_NUM */
#define CFM_TYPE_DFT_TRANSMIT_LBM              1
#define CFM_TYPE_DFT_CCM_PRIORITY              7
#define CFM_TYPE_DFT_DATA_TLV_LEN              60
#define CFM_TYPE_DFT_MA_PRIMARY_VID            0 /*if 0, means has no vlan id*/
#define CFM_TYPE_DFT_MA_NAME_FORMAT            CFM_TYPE_MA_NAME_CHAR_STRING

/* refer to 802.1ag-2007.pdf table  8-9, 8-10
 *                Y.1731.pdf table 10-1
 */
#define CFM_TYPE_DA_MC_CLASS_2                 2 /* i.e. 0x0180c2-000030~37 */
#define CFM_TYPE_DA_MC_CLASS_1                 1 /* i.e. 0x0180c2-000038~3f */

typedef struct CFM_TYPE_PacketHeader_S
{
    UI8_T       dstMac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T       srcMac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T      lport;
    UI16_T      tagInfo;
}CFM_TYPE_PaceketHeader_T;

/*the msg size only has max 16 bytes*/
typedef struct  CFM_TYPE_Msg_S
{
    CFM_TYPE_PaceketHeader_T *packet_header_p;
    L_MM_Mref_Handle_T *mem_ref_p;

}CFM_TYPE_Msg_T;


typedef enum CFM_TYPE_MD_Name_E
{
    CFM_TYPE_MD_NAME_NONE                =VAL_dot1agCfmMdFormat_none,
    CFM_TYPE_MD_NAME_DNS_LIKE_NAME       =VAL_dot1agCfmMdFormat_dnsLikeName,
    CFM_TYPE_MD_NAME_MAC_ADDRESS_AND_UNIT=VAL_dot1agCfmMdFormat_macAddressAndUint,
    CFM_TYPE_MD_NAME_CHAR_STRING         =VAL_dot1agCfmMdFormat_charString,
}CFM_TYPE_MD_Name_T;

typedef enum CFM_TYPE_MA_Name_E
{
    CFM_TYPE_MA_NAME_PRIMARY_VID        =VAL_dot1agCfmMaFormat_primaryVid,
    CFM_TYPE_MA_NAME_CHAR_STRING        =VAL_dot1agCfmMaFormat_charString,
    CFM_TYPE_MA_NAME_UNSING_INT16       =VAL_dot1agCfmMaFormat_unsignedInt16,
    CFM_TYPE_MA_NAME_RFC2856_VPN_ID     =VAL_dot1agCfmMaFormat_rfc2865VpnId,
    CFM_TYPE_MA_NAME_ICC_BASED          =32,    /* for Y.1731 */
}CFM_TYPE_MA_Name_T;

typedef enum CFM_TYPE_MdLevel_E
{
    CFM_TYPE_MD_LEVEL_NONE  =-1,
    CFM_TYPE_MD_LEVEL_0     =MIN_dot1agCfmMdMdLevel,
    CFM_TYPE_MD_LEVEL_1,
    CFM_TYPE_MD_LEVEL_2,
    CFM_TYPE_MD_LEVEL_3,
    CFM_TYPE_MD_LEVEL_4,
    CFM_TYPE_MD_LEVEL_5,
    CFM_TYPE_MD_LEVEL_6,
    CFM_TYPE_MD_LEVEL_7     =MAX_dot1agCfmMdMdLevel
} CFM_TYPE_MdLevel_T;

typedef enum CFM_TYPE_LTM_MdLevel_E
{
    CFM_TYPE_LTM_MD_LEVEL_0=MAX_dot1agCfmMdMdLevel+1,
    CFM_TYPE_LTM_MD_LEVEL_1,
    CFM_TYPE_LTM_MD_LEVEL_2,
    CFM_TYPE_LTM_MD_LEVEL_3,
    CFM_TYPE_LTM_MD_LEVEL_4,
    CFM_TYPE_LTM_MD_LEVEL_5,
    CFM_TYPE_LTM_MD_LEVEL_6,
    CFM_TYPE_LTM_MD_LEVEL_7
} CFM_TYPE_LTM_MdLevel_T;

typedef enum CFM_TYPE_MP_Direction_E
{
    CFM_TYPE_MP_DIRECTION_DOWN =VAL_dot1agCfmMepDirection_down,
    CFM_TYPE_MP_DIRECTION_UP   =VAL_dot1agCfmMepDirection_up,
    CFM_TYPE_MP_DIRECTION_UP_DOWN
}CFM_TYPE_MP_Direction_T;


typedef enum CFM_TYPE_CcmStatus_E
{
    CFM_TYPE_CCM_STATUS_ENABLE  =VAL_dot1agCfmMepCciEnabled_true,
    CFM_TYPE_CCM_STATUS_DISABLE =VAL_dot1agCfmMepCciEnabled_false
}CFM_TYPE_CcmStatus_T;

/*port status 21.5.4*/
typedef enum CFM_TYPE_PortStatus_E
{
    CFM_TYPE_PORT_STATUS_NO_PORT_STATE_TLV     =VAL_dot1agCfmMepDbPortStatusTlv_psNoPortStateTLV,
    CFM_TYPE_PORT_STATUS_BLOCKED               =VAL_dot1agCfmMepDbPortStatusTlv_psBlocked,
    CFM_TYPE_PORT_STATUS_UP                    =VAL_dot1agCfmMepDbPortStatusTlv_psUp
}CFM_TYPE_PortStatus_T;

/*interface status 21.5.5*/
typedef enum CFM_TYPE_InterfaceStatus_E
{
    CFM_TYPE_INTERFACE_STATUS_NO_INTERFACE_STATUS_TLV =VAL_dot1agCfmMepDbInterfaceStatusTlv_isNoInterfaceStatusTLV,
    CFM_TYPE_INTERFACE_STATUS_UP                      =VAL_dot1agCfmMepDbInterfaceStatusTlv_isUp,
    CFM_TYPE_INTERFACE_STATUS_DOWN                    =VAL_dot1agCfmMepDbInterfaceStatusTlv_isDown,
    CFM_TYPE_INTERFACE_STATUS_TESTING                 =VAL_dot1agCfmMepDbInterfaceStatusTlv_isTesting,
    CFM_TYPE_INTERFACE_STATUS_UNKNOWN                 =VAL_dot1agCfmMepDbInterfaceStatusTlv_isUnknown,
    CFM_TYPE_INTERFACE_STATUS_DORMANT                 =VAL_dot1agCfmMepDbInterfaceStatusTlv_isDormant,
    CFM_TYPE_INTERFACE_STATUS_NOTPRESENT              =VAL_dot1agCfmMepDbInterfaceStatusTlv_isNotPresent,
    CFM_TYPE_INTERFACE_STATUS_LOWERLAYERDOWN          =VAL_dot1agCfmMepDbInterfaceStatusTlv_isLowerLayerDown

}CFM_TYPE_InterfaceStatus_T;

typedef enum CFM_TYPE_MhfCreation_E
{
    CFM_TYPE_MHF_CREATION_NONE    =VAL_dot1agCfmMaMhfCreation_defMHFnone,
    CFM_TYPE_MHF_CREATION_DEFAULT =VAL_dot1agCfmMaMhfCreation_defMHFdefault,
    CFM_TYPE_MHF_CREATION_EXPLICIT=VAL_dot1agCfmMaMhfCreation_defMHFexplicit,
    CFM_TYPE_MHF_CREATION_DEFER   =VAL_dot1agCfmMaMhfCreation_defMHFdefer
}CFM_TYPE_MhfCreation_T;

typedef enum CFM_TYPE_CcmInterval_E
{
    CFM_TYPE_CCM_INTERVAL_INVALID  =VAL_dot1agCfmMaCcmInterval_intervalInvalid,
    CFM_TYPE_CCM_INTERVAL_300_HZ   =VAL_dot1agCfmMaCcmInterval_interval300Hz,
    CFM_TYPE_CCM_INTERVAL_10_MS    =VAL_dot1agCfmMaCcmInterval_interval10ms,
    CFM_TYPE_CCM_INTERVAL_100_MS   =VAL_dot1agCfmMaCcmInterval_interval100ms,
    CFM_TYPE_CCM_INTERVAL_1_S      =VAL_dot1agCfmMaCcmInterval_interval1s,
    CFM_TYPE_CCM_INTERVAL_10_S     =VAL_dot1agCfmMaCcmInterval_interval10s,
    CFM_TYPE_CCM_INTERVAL_1_MIN    =VAL_dot1agCfmMaCcmInterval_interval1min,
    CFM_TYPE_CCM_INTERVAL_10_MIN   =VAL_dot1agCfmMaCcmInterval_interval10min
}CFM_TYPE_CcmInterval_T;

typedef enum CFM_TYPE_FNG_State_E
{
    CFM_TYPE_FNG_STATE_RESET   =VAL_dot1agCfmMepFngState_fngReset,
    CFM_TYPE_FNG_STATE_DEFECT  =VAL_dot1agCfmMepFngState_fngDefect,
    CFM_TYPE_FNG_STATE_REPORTED=VAL_dot1agCfmMepFngState_fngDefectReported,
    CFM_TYPE_FNG_STATE_CLERING =VAL_dot1agCfmMepFngState_fngDefectClearing
}CFM_TYPE_FNG_State_T;

/*RDI>MAC>REMOTE>ERROR>XCON*/
typedef enum CFM_TYPE_FNG_HighestDefectPri_E
{
    CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE        =VAL_dot1agCfmMepHighestPrDefect_none,
    CFM_TYPE_FNG_HIGHEST_DEFECT_RDI_CCM         =VAL_dot1agCfmMepHighestPrDefect_defRDICCM,
    CFM_TYPE_FNG_HIGHEST_DEFECT_MAC_STATUS      =VAL_dot1agCfmMepHighestPrDefect_defMACstatus,
    CFM_TYPE_FNG_HIGHEST_DEFECT_REMOTE_CCM      =VAL_dot1agCfmMepHighestPrDefect_defRemoteCCM,
    CFM_TYPE_FNG_HIGHEST_DEFECT_ERROR_CCM       =VAL_dot1agCfmMepHighestPrDefect_defErrorCCM,
    CFM_TYPE_FNG_HIGHEST_DEFECT_XCON_CCM        =VAL_dot1agCfmMepHighestPrDefect_defXconCCM

}CFM_TYPE_FNG_HighestDefectPri_T;

typedef enum CFM_TYPE_FNG_LowestAlarmPri_E
{
    CFM_TYPE_FNG_LOWEST_ALARM_ALL              =VAL_dot1agCfmMepLowPrDef_allDef,
    CFM_TYPE_FNG_LOWEST_ALARM_MAC_REM_ERR_XCON =VAL_dot1agCfmMepLowPrDef_macRemErrXcon,
    CFM_TYPE_FNG_LOWEST_ALARM_REM_ERR_XCON     =VAL_dot1agCfmMepLowPrDef_remErrXcon,
    CFM_TYPE_FNG_LOWEST_ALARM_ERR_XCON         =VAL_dot1agCfmMepLowPrDef_errXcon,
    CFM_TYPE_FNG_LOWEST_ALARM_XCON             =VAL_dot1agCfmMepLowPrDef_xcon,
    CFM_TYPE_FNG_LOWEST_ALARM_NO_DEF           =VAL_dot1agCfmMepLowPrDef_noXcon

}CFM_TYPE_FNG_LowestAlarmPri_T;

typedef enum CFM_TYPE_RelayAction_E /*21.9.5*/
{
    CFM_TYPE_RELAY_ACTION_HIT  =VAL_dot1agCfmLtrRelay_rlyHit,
    CFM_TYPE_RELAY_ACTION_FDB  =VAL_dot1agCfmLtrRelay_rlyFdb,
    CFM_TYPE_RELAY_ACTION_MPDB =VAL_dot1agCfmLtrRelay_rlyMpdb
}CFM_TYPE_RelayAction_T;

typedef enum CFM_TYPE_IngressAction_E /*21.8.8.1*/
{
    CFM_TYPE_INGRESS_ACTION_OK     =VAL_dot1agCfmLtrIngress_ingOk,
    CFM_TYPE_INGRESS_ACTION_DOWN   =VAL_dot1agCfmLtrIngress_ingDown,
    CFM_TYPE_INGRESS_ACTION_BLOCKED=VAL_dot1agCfmLtrIngress_ingBlocked,
    CFM_TYPE_INGRESS_ACTION_VID    =VAL_dot1agCfmLtrIngress_ingVid
}CFM_TYPE_IngressAction_T ;

typedef enum CFM_TYPE_EgressAaction_E /*21.9.9.1*/
{
    CFM_TYPE_EGRESS_ACTION_OK     =VAL_dot1agCfmLtrEgress_egrOK,
    CFM_TYPE_EGRESS_ACTION_DOWN   =VAL_dot1agCfmLtrEgress_egrDown,
    CFM_TYPE_EGRESS_ACTION_BLOCKED=VAL_dot1agCfmLtrEgress_egrBlocked,
    CFM_TYPE_EGRESS_ACTION_VID    =VAL_dot1agCfmLtrEgress_egrVid
}CFM_TYPE_EgressAction_T;

typedef enum CFM_TYPE_RemoteMepSate_E
{
    CFM_TYPE_REMOTE_MEP_STATE_IDLE  =VAL_dot1agCfmMepDbRMepState_rMepIdle,
    CFM_TYPE_REMOTE_MEP_STATE_START =VAL_dot1agCfmMepDbRMepState_rMepStart,
    CFM_TYPE_REMOTE_MEP_STATE_FAILD =VAL_dot1agCfmMepDbRMepState_rMepFailed,
    CFM_TYPE_REMOTE_MEP_STATE_OK    =VAL_dot1agCfmMepDbRMepState_rMepOk
}CFM_TYPE_RemoteMepState_T;


typedef enum CFM_TYPE_MEP_ConfigError_E
{
    /*mib define*/
    CFM_TYPE_CONFIG_ERROR_LEAK              =VAL_dot1agCfmConfigErrorListErrorType_cfmLeak,
    CFM_TYPE_CONFIG_ERROR_VIDS              =VAL_dot1agCfmConfigErrorListErrorType_conflictingVids,
    CFM_TYPE_CONFIG_ERROR_EXCESSIVE_LEVELS  =VAL_dot1agCfmConfigErrorListErrorType_excessiveLevels,
    CFM_TYPE_CONFIG_ERROR_OVERLAPPED_LEVELS =VAL_dot1agCfmConfigErrorListErrorType_overlappedLevels,
    CFM_TYPE_CONFIG_ERROR_AIS
}CFM_TYPE_MEP_ConfigError_T;


typedef enum CFM_TYPE_Config_E
{
    CFM_TYPE_CONFIG_ERROR=FALSE,
    CFM_TYPE_CONFIG_SUCCESS=TRUE,

}CFM_TYPE_Config_T;

typedef enum CFM_TYPE_TlvType_E
{
    CFM_TYPE_TLV_END              = 0,
    CFM_TYPE_TLV_SENDER_ID        = 1,
    CFM_TYPE_TLV_PORT_STATUS      = 2,
    CFM_TYPE_TLV_DATA             = 3,
    CFM_TYPE_TLV_INTERFACE_STATUS= 4,
    CFM_TYPE_TLV_REPLY_INGRESS    = 5,
    CFM_TYPE_TLV_REPLY_EGRESS     = 6,
    CFM_TYPE_TLV_LTM_EGRESS_ID    = 7,
    CFM_TYPE_TLV_LTR_EGRESS_ID    = 8,
    CFM_TYPE_TLV_ORGANIZATION     = 31,
    CFM_TYPE_TLV_BY_ITU_T_Y_1731  = 32,
    CFM_TYPE_TLV_RESERV           = 64
}CFM_TYPE_TlvType_T;

/*CFM chassis id subtype enumeration, the same as CFM*/
typedef enum CFM_TYPE_TlvChassisIdSubtype_E
{
    CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_CHASSIS        =VAL_dot1agCfmMepDbChassisIdSubtype_chassisComponent,
    CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_IFALIAS        =VAL_dot1agCfmMepDbChassisIdSubtype_interfaceAlias,
    CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_PORT           =VAL_dot1agCfmMepDbChassisIdSubtype_portComponent,
    CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_MAC_ADDR       =VAL_dot1agCfmMepDbChassisIdSubtype_macAddress,
    CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_NETWORK_ADDR   =VAL_dot1agCfmMepDbChassisIdSubtype_networkAddress,
    CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_IFNAME         =VAL_dot1agCfmMepDbChassisIdSubtype_interfaceName,
    CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_LOCAL          =VAL_dot1agCfmMepDbChassisIdSubtype_local,

    CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_RESERVED
}CFM_TYPE_TlvChassisIdSubtype_T;


typedef enum CFM_TYPE_MhfIdPermission_E
{
    CFM_TYPE_MHF_ID_PERMISSION_NONE                  =VAL_dot1agCfmMdMhfIdPermission_sendIdNone,
    CFM_TYPE_MHF_ID_PERMISSION_CHASSIS               =VAL_dot1agCfmMdMhfIdPermission_sendIdChassis,
    CFM_TYPE_MHF_ID_PERMISSION_SENDID_MANAGE         =VAL_dot1agCfmMdMhfIdPermission_sendIdManage,
    CFM_TYPE_MHF_ID_PERMISSION_SENDID_CHASSIS_MANAGE=VAL_dot1agCfmMdMhfIdPermission_sendIdChassisManage,
    CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER          =VAL_dot1agCfmMdMhfIdPermission_sendIdDefer
}CFM_TYPE_MhfIdPermission_T;

/* CFM port id subtype enumeration*/
typedef enum CFM_TYPE_PortIdSubType_E
{
    CFM_TYPE_PORT_ID_SUBTYPE_IFALIAS          =VAL_dot1agCfmLtrEgressPortIdSubtype_interfaceAlias,
    CFM_TYPE_PORT_ID_SUBTYPE_PORT             =VAL_dot1agCfmLtrEgressPortIdSubtype_portComponent,
    CFM_TYPE_PORT_ID_SUBTYPE_MAC_ADDR         =VAL_dot1agCfmLtrEgressPortIdSubtype_macAddress,
    CFM_TYPE_PORT_ID_SUBTYPE_NETWORK_ADDR     =VAL_dot1agCfmLtrEgressPortIdSubtype_networkAddress,
    CFM_TYPE_PORT_ID_SUBTYPE_IFNAME           =VAL_dot1agCfmLtrEgressPortIdSubtype_interfaceName,
    CFM_TYPE_PORT_ID_SUBTYPE_AGENT_CIRCUIT_ID =VAL_dot1agCfmLtrEgressPortIdSubtype_agentCircuitId,
    CFM_TYPE_PORT_ID_SUBTYPE_LOCAL            =VAL_dot1agCfmLtrEgressPortIdSubtype_local,

    CFM_TYPE_PORT_ID_SUBTYPE_RESERVED
}CFM_TYPE_PortIdSubtype_T;

typedef enum CFM_TYPE_OrganizationSubtype_E
{
    CFM_TYPE_ORGANIZATION_SUBTYPE_RESERVED=0,
    CFM_TYPE_ORGANIZATION_SUBTYPE_PORT_VLAN_ID,
    CFM_TYPE_ORGANIZATION_SUBTYPE_PORT_PROTOCOL_VLAN_ID,
    CFM_TYPE_ORGANIZATION_SUBTYPE_VLAN_NAME,
    CFM_TYPE_ORGANIZATION_SUBTYPE_PROTOCOL_ID
}CFM_TYPE_OrganizationSubtype_T;

typedef enum CFM_TYPE_OpCode_E
{
    CFM_TYPE_OPCODE_RESERVE =0,
    CFM_TYPE_OPCODE_CCM     =1,
    CFM_TYPE_OPCODE_LBR     =2,
    CFM_TYPE_OPCODE_LBM     =3 ,
    CFM_TYPE_OPCODE_LTR     =4,
    CFM_TYPE_OPCODE_LTM     =5,
    CFM_TYPE_OPCODE_AIS     =33,
    CFM_TYPE_OPCODE_DMR     =46,
    CFM_TYPE_OPCODE_DMM     =47,
    CFM_TYPE_OPCODE_LEGAL
}CFM_TYPE_OpCode_T;

typedef enum CFM_TYPE_LinktraceStatus_E
{
    CFM_TYPE_LINKTRACE_STATUS_ENABLE =1,
    CFM_TYPE_LINKTRACE_STATUS_DISABLE
}CFM_TYPE_LinktraceStatus_T;

typedef enum CFM_TYPE_CfmStatus_E
{
    CFM_TYPE_CFM_STATUS_ENABLE =1,
    CFM_TYPE_CFM_STATUS_DISABLE
}CFM_TYPE_CfmStatus_T;


typedef enum CFM_TYPE_CrossCheckStatus_E
{
    CFM_TYPE_CROSS_CHECK_STATUS_DISABLE,
    CFM_TYPE_CROSS_CHECK_STATUS_ENABLE

}CFM_TYPE_CrossCheckStatus_T;

typedef enum CFM_TYPE_SnmpTrapsCC_E
{
    CFM_TYPE_SNMP_TRAPS_CC_MEP_UP,
    CFM_TYPE_SNMP_TRAPS_CC_MEP_DOWN,
    CFM_TYPE_SNMP_TRAPS_CC_CONFIG,
    CFM_TYPE_SNMP_TRAPS_CC_LOOP,
    CFM_TYPE_SNMP_TRAPS_CC_ALL
}CFM_TYPE_SnmpTrapsCC_T;

typedef enum CFM_TYPE_SnmpTrapsCrossCheck_E
{
    CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_UNKNOWN,
    CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_MISSING,
    CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MA_UP,
    CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_ALL
}CFM_TYPE_SnmpTrapsCrossCheck_T;

typedef enum CFM_TYPE_MpType_E
{
    CFM_TYPE_MP_TYPE_NONE,
    CFM_TYPE_MP_TYPE_MEP,
    CFM_TYPE_MP_TYPE_MIP
}CFM_TYPE_MpType_T;

typedef enum CFM_TYPE_TxReason_E
{
    CFM_TYPE_XMITPDU,
    CFM_TYPE_FLOODPDU,
    CFM_TYPE_XMITCCM,
    CFM_TYPE_XMITLBM,
    CFM_TYPE_XMITLBR,
    CFM_TYPE_DOTHRPTMEASUREBYLBM,
    CFM_TYPE_XMITLTM,
    CFM_TYPE_XMITLTR,
    CFM_TYPE_XMITAIS,
    CFM_TYPE_XMITDMM,
    CFM_TYPE_XMITDMR,
    CFM_TYPE_ALLOCATE_MEP,                    /*allocate mep space*/
    CFM_TYPE_ALLOCATE_RMEP,                    /*allocate remote mep space*/
    CFM_TYPE_ALLOCATE_LTR,                        /*allocate mep space*/
    CFM_TYPE_TIMER,                                   /*allocate timer space*/
    CFM_TYPE_TIMER_PARA
}CFM_TYPE_TxReason_T;


typedef enum CFM_TYPE_AisStatus_E
{
    CFM_TYPE_AIS_STATUS_ENABLE,
    CFM_TYPE_AIS_STATUS_DISABLE
}CFM_TYPE_AIS_STATUS_T;

#define CFM_TYPE_AIS_PERIOD_1S  1
#define CFM_TYPE_AIS_PERIOD_60S 60

/* for delay measurement
 */
/* according to IEEE 1588-2002
 */
typedef struct
{
    UI32_T  secs;
    UI32_T  nano_secs;
}CFM_TYPE_TimeReprFmt_T;

enum
{
    CFM_TYPE_DMM_REPLY_REC_STATE_NONE =0,
    CFM_TYPE_DMM_REPLY_REC_STATE_TIMEOUT,
    CFM_TYPE_DMM_REPLY_REC_STATE_SENDFAIL,
    CFM_TYPE_DMM_REPLY_REC_STATE_RECEIVED,
    CFM_TYPE_DMM_REPLY_REC_STATE_WAITREPLY,
    CFM_TYPE_DMM_REPLY_REC_STATE_WRONGPKTLEN,
};

/* for throughput measurement
 */
enum
{
    CFM_TYPE_THRPT_MEASURE_RES_SENDPKTS      = 0x1,
    CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_1SEC = 0x2,
    CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_TOUT = 0x4,
};
#endif /*#if (SYS_CPNT_CFM == TRUE)*/
#endif /*END CFM_TYPE_H*/
