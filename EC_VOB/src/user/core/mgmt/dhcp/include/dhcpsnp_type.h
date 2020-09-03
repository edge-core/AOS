/* Module Name:	DHCPSNP_TYPE.H
 * Purpose:
 *		This module declares all structures used in DHCPSNP, structure name and
 *		field name do not be changed.
 *
 * Notes:
 *
 * History:
 *		 Date		-- Modifier,  Reason
 *  0.1 2006.1.16  --  Andrew Huang, Created
 *
 * Copyright(C)		 Accton	Corporation, 1999, 2000, 2001.
 */
 
#ifndef _DHCPSNP_TYPE_H
#define _DHCPSNP_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "l_mm.h"
#include "leaf_es3626a.h"
#include "dhcp_type.h"
//#include "l_mem.h"          /* 2006-07, Joseph */

/* Function returned value 
 */
#define DHCPSNP_TYPE_OK                                     0x80000001
#define DHCPSNP_TYPE_FAIL                                   0x80000002
#define DHCPSNP_TYPE_INVALID_ARG                            0x80000003
#define DHCPSNP_TYPE_INVALID_RANGE                          0x80000004
#define DHCPSNP_TYPE_BINDING_TABLE_FULL                     0x80000005
#define DHCPSNP_TYPE_BINDING_ENTRY_NOT_EXISTED              0x80000006
#define DHCPSNP_TYPE_NO_MORE_MEMORY                         0x80000007
#define DHCPSNP_TYPE_NO_MORE_ENTRY                          0x80000008
#define DHCPSNP_TYPE_DROP_DHCP_PACKET                       0x80000009
#define DHCPSNP_TYPE_EXCEED_MAX_NBR_OF_CLIENT_PER_PORT      0x8000000A
#define DHCPSNP_TYPE_FAIL_TO_DELETE_STATIC_ENTRY            0x8000000B
#define DHCPSNP_TYPE_MY_MAC                                 0x80000010
#define DHCPSNP_TYPE_UNICAST                                0x80000020
#define DHCPSNP_TYPE_BROADCAST                              0x80000030
#define DHCPSNP_TYPE_DROP                                   0x80000040

/* NAME CONSTANT DECLARATIONS
 */
/* A complier option only for developed test*/
#define DHCPSNP_TYPE_DEBUG                          TRUE

/* DHCP Snooping Global Status, default is disabled. */
#define DHCPSNP_TYPE_SNOOPING_GLOBAL_DISABLED       VAL_dhcpSnoopEnable_disabled
#define DHCPSNP_TYPE_SNOOPING_GLOBAL_ENABLED        VAL_dhcpSnoopEnable_enabled
#define DHCPSNP_TYPE_SNOOPING_GLOBAL_DEFAULT        SYS_DFLT_DHCPSNP_GLOBAL

/* DHCP Snooping Status Per VLAN, default is disabled. */
#define DHCPSNP_TYPE_SNOOPING_VLAN_DISABLED         VAL_dhcpSnoopVlanEnable_disabled
#define DHCPSNP_TYPE_SNOOPING_VLAN_ENABLED          VAL_dhcpSnoopVlanEnable_enabled
#define DHCPSNP_TYPE_SNOOPING_VLAN_DEFAULT          SYS_DFLT_DHCPSNP_VLAN

/* DHCP Snooping Verify Mac Address Status, default is enabled. */
#define DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_ENABLED     VAL_dhcpSnoopVerifyMacAddressEnable_enabled
#define DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_DISABLED    VAL_dhcpSnoopVerifyMacAddressEnable_disabled
#define DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_DEFAULT     SYS_DFLT_DHCPSNP_VERIFY_MAC_ADDRESS

/* The MIB format type of the Binding Entry */
#define DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY     VAL_dhcpSnoopBindingsEntryType_dynamic  /* could be used by DHCP SNP & IP Src Guard  */
#define DHCPSNP_TYPE_STA_IP_SRC_GRD_BINDING_ENTRY   VAL_ipSrcGuardBindingsEntryType_static  /* only used by IP Src Guard when it's enabled */
#if (SYS_CPNT_BOOTP == TRUE)
#define DHCPSNP_TYPE_DYN_BOOTP_BINDING_ENTRY        4L   
#endif

/* The trust status of the port that would used in DHCP Snooping, default is untrusted */
#define DHCPSNP_TYPE_PORT_UNTRUSTED                 VAL_dhcpSnoopPortTrustEnable_disabled
#define DHCPSNP_TYPE_PORT_TRUSTED                   VAL_dhcpSnoopPortTrustEnable_enabled
#define DHCPSNP_TYPE_PORT_TRUST_STATUS_DEFAULT      SYS_DFLT_DHCPSNP_PORT_TRUST_MODE


/* The config type of a port */
#define DHCPSNP_TYPE_PORT_TRUST_CONFIGED            BIT_0
#define DHCPSNP_TYPE_PORT_LIMIT_RATE_CONFIGED       BIT_1


/* begin 2006-07, Joseph */
/* The Option82 Status of DHCP Snooping, default is disabled */
#define DHCPSNP_TYPE_OPTION82_ENABLED               VAL_dhcpSnoopInformationOptionEnable_enabled
#define DHCPSNP_TYPE_OPTION82_DISABLED              VAL_dhcpSnoopInformationOptionEnable_disabled
#define DHCPSNP_TYPE_OPTION82_STATUS_DEFAULT        SYS_DFLT_DHCPSNP_OPTION82_STATUS_DEFAULT
#define DHCPSNP_TYPE_OPTION82_POLICY_DROP           VAL_dhcpSnoopInformationOptionPolicy_drop
#define DHCPSNP_TYPE_OPTION82_POLICY_KEEP           VAL_dhcpSnoopInformationOptionPolicy_keep
#define DHCPSNP_TYPE_OPTION82_POLICY_REPLACE        VAL_dhcpSnoopInformationOptionPolicy_replace
#define DHCPSNP_TYPE_OPTION82_POLICY_DEFAULT        SYS_DFLT_DHCPSNP_OPTION82_POLICY_DEFAULT
/* end 2006-07 */

#define DHCPSNP_TYPE_L2_RELAY_FORWARDING_DEFAULT    TRUE
#define DHCPSNP_TYPE_L3_RELAY_FORWARDING_DEFAULT    TRUE

/* The size of the vlan bitmap table mapped with dhcp snooping status. */
#define DHCPSNP_TYPE_VLAN_BITMAP_SIZE               4096
#define DHCPSNP_TYPE_VLAN_BITMAP_SIZE_IN_BYTE       DHCPSNP_TYPE_VLAN_BITMAP_SIZE/8

/* The max binding entries including dhcp snooping and ip source guard in the system */
#define DHCPSNP_TYPE_MAX_NBR_OF_CLIENT_PER_PORT     SYS_ADPT_DHCPSNP_MAX_NBR_OF_CLIENT_PER_PORT
#define DHCPSNP_TYPE_MAX_NBR_OF_BINDING_ENTRY       SYS_ADPT_DHCPSNP_MAX_NBR_OF_BINDING_ENTRY

#define DHCPSNP_TYPE_KEY_LEN                    (SYS_ADPT_MAC_ADDR_LEN + SYS_ADPT_IPV4_ADDR_LEN + 5)      /* lporpt_ifindex(4)+ type(1) + ip(4)+ mac(6) */

#define DHCPSNP_TYPE_SYSTEM_NO_RATELIMIT            0
#define UDP_HEADER_LEN                              8

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_TR101_FORMAT == TRUE)
#define DHCPSNP_TYPE_OPTION82_TR101_NO_BOARD_ID     0xFF
#endif

typedef enum
{
    /* DHCPSNP snooping binding key index 
     */
    DHCPSNP_TYPE_PRI_KEY_INDEX =0,      /* ip_addr + mac_addr */
    DHCPSNP_TYPE_SEC_KEY_INDEX,         /* lport_ifindex + type + ip_addr + mac_addr */
} DHCPSNP_TYPE_KEY_INDEX_T;


typedef enum
{
    DHCPSNP_TYPE_OPTION82_RID_MAC_HEX =1,
    DHCPSNP_TYPE_OPTION82_RID_MAC_ASCII,
    DHCPSNP_TYPE_OPTION82_RID_IP_HEX,
    DHCPSNP_TYPE_OPTION82_RID_IP_ASCII,
    DHCPSNP_TYPE_OPTION82_RID_CONFIGURED_STRING,
#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_TR101_FORMAT == TRUE)
    DHCPSNP_TYPE_OPTION82_RID_TR101_IP,
    DHCPSNP_TYPE_OPTION82_RID_TR101_SYSNAME,
#endif
#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_RID_SUB_OPTION == TRUE)
    DHCPSNP_TYPE_OPTION82_RID_CONFIGURED_STRING_PLUS_PORT_DESCRIPTION,
#endif
    DHCPSNP_TYPE_OPTION82_RID_MAX,
} DHCPSNP_TYPE_OPTION82_RID_E;

typedef enum
{
    DHCPSNP_TYPE_OPTION82_CID_VLAN_UNIT_PORT =1,
    DHCPSNP_TYPE_OPTION82_CID_CONFIGURED_STRING,
#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_TR101_FORMAT == TRUE)
    DHCPSNP_TYPE_OPTION82_CID_TR101_IP,
    DHCPSNP_TYPE_OPTION82_CID_TR101_SYSNAME,
#endif
    DHCPSNP_TYPE_OPTION82_CID_MAX,
} DHCPSNP_TYPE_OPTION82_CID_E;

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    DHCPSNP_TYPE_TRACE_ID_DHCPSNP_ENGINE_INSERTOPTION82ANDFORWARD = 0,
    DHCPSNP_TYPE_TRACE_ID_DHCPSNP_ENGINE_REMOVEOPTION82ANDFORWARD,
    DHCPSNP_TYPE_TRACE_ID_DHCPSNP_ENGINE_CLONE_MREF,
};

typedef enum
{
    DHCPSNP_TYPE_SYSTEM_STATE_TRANSITION    = SYS_TYPE_STACKING_TRANSITION_MODE,
    DHCPSNP_TYPE_SYSTEM_STATE_MASTER        = SYS_TYPE_STACKING_MASTER_MODE,
    DHCPSNP_TYPE_SYSTEM_STATE_SLAVE         = SYS_TYPE_STACKING_SLAVE_MODE,
    DHCPSNP_TYPE_SYSTEM_STATE_PROVISION_COMPLETE,
    DHCPSNP_TYPE_SYSTEN_STATE_RUNNING
} DHCPSNP_TYPE_SYSTEM_STATE_T;

/* DHCP packet classification type */
typedef enum DHCPSNP_TYPE_PKT_CLASSIFICATION_TYPE_E
{
    DHCPSNP_TYPE_CLIENT_PKT,
    DHCPSNP_TYPE_SERVER_PKT
} DHCPSNP_TYPE_PKT_CLASSIFICATION_TYPE_T;

/* Internal use binding type, for database stored and search
 */
typedef enum DHCPSNP_TYPE_InternalBindingType_E
{
    DHCPSNP_TYPE_INTERNAL_BINDING_TYPE_IPSG = 1,
    DHCPSNP_TYPE_INTERNAL_BINDING_TYPE_DHCPSNP,
#if (SYS_CPNT_BOOTP == TRUE)    
    DHCPSNP_TYPE_INTERNAL_BINDING_TYPE_BOOTP,
#endif    
} DHCPSNP_TYPE_InternalBindingType_T;

typedef enum /* function number */
{
	DHCPSNP_TYPE_ENGINE_RX_PROCESS_PKT = 0,
	DHCPSNP_TYPE_ENGINE_ADD_DYNAMIC_BINDING,
    DHCPSNP_TYPE_ENGINE_IP_MAC_BINDING_CHECK,
} DHCPSNP_TYPE_Function_Number_T ;

typedef enum /* error code */
{
	DHCPSNP_TYPE_PKT_LEN_TOO_SHORT = 0,
	DHCPSNP_TYPE_FAIL_TO_CLASSIFY,
	DHCPSNP_TYPE_NOT_MATCH_BINDING,
	DHCPSNP_TYPE_FAIL_TO_DEL_BINDING,
	DHCPSNP_TYPE_SA_NOT_MATCH_CLIENT_HW_ADDR,
	DHCPSNP_TYPE_FAIL_TO_FIND_FWD_PORT,
	DHCPSNP_TYPE_NOT_VLAN_MEMBER,
	DHCPSNP_TYPE_FAIL_TO_ADD_BINDING,
    DHCPSNP_TYPE_BLOCK_RECORD_DROP,
} DHCPSNP_TYPE_Error_Number_T;



/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */




typedef void (*DHCPSNP_TYPE_RxDhcpPktHandler_T)(L_MM_Mref_Handle_T *mem_ref_p, UI32_T packet_length, UI32_T rxIfNum, 
                                   UI8_T *dst_mac_p, UI8_T *src_mac_p, UI32_T vid, UI32_T src_lport_ifIndex);
             
typedef  struct DHCPSNP_TYPE_BindingEntry_S
{
    UI32_T  ip_addr;                /*primary key */
    UI32_T  ip_addr_type;           /*ipv4,ipv6,dns...etc */
    UI32_T  lease_start_time;       /*the system total seconds that the binding entry is created*/
    UI32_T  lease_time;             
    UI32_T  vid_ifindex;
    UI32_T  lport_ifindex;
    UI32_T  row_status;
    UI32_T  vid;                    /* patch temporarily for MIB format */
    UI8_T   mac_p[SYS_ADPT_MAC_ADDR_LEN];                   /*primary key*/
    UI8_T   type;                   /*could be one of 3 types of binding entry. */
} DHCPSNP_TYPE_BindingEntry_T;

typedef  struct DHCPSNP_TYPE_PortInfo_S
{
    UI32_T  binding_num;            /* current binding number on the port */  
    BOOL_T  trust_status;           /* DHCPSNP_TYPE_PORT_UNTRUSTED or DHCPSNP_TYPE_PORT_TRUSTED */
    UI8_T   option82_cid_mode;      /* DHCPSNP_TYPE_OPTION82_CID_VLAN_UNIT_PORT or DHCPSNP_TYPE_OPTION82_CID_CONFIGURED_STRING*/
    UI8_T   option82_cid_value[SYS_ADPT_MAX_LENGTH_OF_CID + 1];
} DHCPSNP_TYPE_PortInfo_T;

typedef struct DHCPSNP_TYPE_StatisticCounter_S
{
    UI32_T fwd_cnt;                 /* total forward counter */
    UI32_T untrusted_drop_cnt;      /* drop from untrusted port counter */
    UI32_T fwd_port_err_cnt;        /* drop due to no forwarding port counter */
    UI32_T add_err_cnt;             /* drop due to add dynamic binding failed counter */
} DHCPSNP_TYPE_StatisticCounter_T;

#if (SYS_CPNT_DHCPSNP_SYSTEM_RATELIMIT == TRUE)
typedef struct DHCPSNP_TYPE_Rate_S
{
	UI32_T	count;		/* packet count in time interval */
	UI32_T	rate;		/* result rate in last time interval */
} DHCPSNP_TYPE_Rate_T;
#endif


typedef struct DHCPSNP_TYPE_RecvPkt_S
{
	UI8_T                 dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                 src_mac[SYS_ADPT_MAC_ADDR_LEN];	
    UI32_T                pkt_len;
    UI32_T                ing_vid;
    UI32_T                ing_lport;
	L_MM_Mref_Handle_T    *mem_ref_p;
	DHCP_TYPE_IpHeader_T  *ip_hdr_p;
	DHCP_TYPE_UdpHeader_T *udp_hdr_p;
	struct dhcp_packet    *dhcp_hdr_p;   
} DHCPSNP_TYPE_RecvPkt_T;
#endif	


