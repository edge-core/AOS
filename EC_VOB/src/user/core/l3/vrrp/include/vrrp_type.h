/* -------------------------------------------------------------------------------------
 * FILE	NAME: VRRP_TYPES.h                                                                 
 *                                                                                      
 * PURPOSE: This package provides the data types used in VRRP (RFC 2338)
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009                           
 * -------------------------------------------------------------------------------------*/


#ifndef __VRRP_TYPE_H
#define __VRRP_TYPE_H

#include "sys_adpt.h"
#include "sys_type.h"
#include "l_sort_lst.h"
//#include "l_mref.h"
#include "leaf_2787.h"

#define VRRP_TYPE_DFLT_TIMER_TICKS         SYS_BLD_TICKS_PER_SECOND 
#define VRRP_TYPE_NO_EXPIRE_TIME           0xFFFFFFFF

/* Ahthentication data Length*/
#define VRRP_MGR_AUTH_DATA_LENGTH          SYS_ADPT_VRRP_AUTHENTICATION_KEY_LEN
#define VRRP_IP_ADDR_LENGTH                4

#define VRRP_PROTOCOL_NUMBER            112
#define VRRP_ONE_SECOND_TICKS           100 	/* 1 sec = 100 ticks */
#define VRRP_GROUP_MAC_ADDR             0x01005e000012

#define VRRP_ETHER_FRAME_IP_TYPE        0x0800
#define VRRP_ETHER_FRAME_ARP_TYPE       0x0806

#define VRRP_IP_TTL                     255
#define VRRP_VERSION                    2
#define VRRP_ADVER_TYPE                 1

#define VRRP_TYPE_MAX_PRIORITY			MAX_vrrpOperPriority
#define VRRP_TYPE_MIN_PRIORITY			MIN_vrrpOperPriority

#define VRRP_DEFAULT_OPER_PROTOCOL		VAL_vrrpOperProtocol_ip
#define VRRP_DEFAULT_IP_ADDR_COUNT		0

#define VRRP_TYPE_SEC_TO_TICK(sec)      ((sec)*SYS_BLD_TICKS_PER_SECOND)
#define VRRP_TYPE_MIN_SYS_TICK          1
enum
{
    VRRP_TYPE_RESULT_SUCCESS = 0,
    VRRP_TYPE_RESULT_FAIL
};

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    VRRP_TYPE_TRACE_ID_VRRP_SEND_ADVERTISEMENT = 0,
    VRRP_TYPE_TRACE_ID_VRRP_SEND_GRATUITOUS_ARP
};

enum 
{
    L_MM_TX_BUF_POOL_ID_VRRP_STKTPLG,
    L_MM_TX_BUF_POOL_ID_VRRP_TX,
};

/* Function number */
enum
{
    VRRP_TYPE_VM_STARTUP = 0,
    VRRP_TYPE_VM_SHUTDOWN,
    VRRP_TYPE_VM_CHECK_MASTER_DOWN_TIMER,
    VRRP_TYPE_VM_PROCESS_VRRP_PKT,
    VRRP_TYPE_TXRX_SEND_ADVERTISEMENT,
    VRRP_TYPE_TXRX_BUILD_AND_SEND_GRAARP,
    VRRP_TYPE_TXRX_SEND_PKT,
};

/* Error number */
enum
{
    VRRP_TYPE_EH_UNKNOWN = 0,
    VRRP_TYPE_EH_STATE_CHANGE,
    VRRP_TYPE_EH_RX_PKT_DA_ERROR,
    VRRP_TYPE_EH_GET_PDU_ERROR,
    VRRP_TYPE_EH_MISCONFIGURATION,
    VRRP_TYPE_EH_LMREF_ERROR,
    VRRP_TYPE_EH_MAX_NUMBER,
};

typedef enum VRRP_TYPE_PingStatus_E
{
	VRRP_TYPE_PING_STATUS_ENABLE = 1,
	VRRP_TYPE_PING_STATUS_DISABLE
}VRRP_TYPE_PingStatus_T;


typedef enum VRRP_TYPE_ROUTER_STATISTICS_FLAG_E
{
    VRRP_ROUTER_CHECKSUM_ERROR = 1,
    VRRP_ROUTER_VERSION_ERROR,
    VRRP_ROUTER_VRID_ERROR,
} VRRP_TYPE_ROUTER_STATISTICS_FLAG_T;

/* TYPE DECLARATIONS 
 */

typedef struct VRRP_OM_Router_Statistics_Info_S
{
    UI32_T                              vrrpRouterChecksumErrors;
    UI32_T                              vrrpRouterVersionErrors;
    UI32_T                              vrrpRouterVrIdErrors;
} VRRP_OM_Router_Statistics_Info_T;

typedef struct
{
    UI32_T                              vrrpStatsBecomeMaster;
    UI32_T                              vrrpStatsAdvertiseRcvd;
    UI32_T                              vrrpStatsAdvertiseIntervalErrors;
    UI32_T                              vrrpStatsAuthFailures;
    UI32_T                              vrrpStatsIpTtlErrors;
    UI32_T                              vrrpStatsPriorityZeroPktsRcvd;
    UI32_T                              vrrpStatsPriorityZeroPktsSent;
    UI32_T                              vrrpStatsInvalidTypePktsRcvd;
    UI32_T                              vrrpStatsAddressListErrors;
    UI32_T                              vrrpStatsInvalidAuthType;
    UI32_T                              vrrpStatsAuthTypeMismatch;
    UI32_T                              vrrpStatsPacketLengthErrors;
} VRRP_OM_Vrrp_Statistics_Info_T;

typedef struct VRRP_TYPE_GlobalEntry_S
{
	UI32_T		vrrpPingStatus;
} VRRP_TYPE_GlobalEntry_T;

typedef struct VRRP_OPER_ENTRY_S
{
    UI32_T      ifindex;
    UI8_T       vrid;
    UI8_T       virtual_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T      oper_state;
    UI32_T      admin_state;
    UI32_T      priority;
    UI32_T      pre_priority;
    UI32_T      ip_addr_count;
    UI8_T       master_ip_addr[4];
    UI8_T       primary_ip_addr[4];
    UI32_T      auth_type;
    UI8_T       auth_key[VRRP_MGR_AUTH_DATA_LENGTH+1];
    UI32_T      advertise_interval;
    UI32_T      preempt_mode;
    UI32_T      preempt_delay;
    BOOL_T      preempt_delay_start;
    UI32_T      virtual_router_up_time;
    UI32_T      oper_protocol;
    UI32_T      row_status;
    UI32_T      master_priority;
    UI32_T      master_advertise_int;
    UI32_T      master_down_int;
    UI32_T      transmit_expire;     //in ticks
    UI32_T      master_down_expire;  //in ticks
    UI32_T      preempt_delay_expire;//in ticks
    UI32_T      owner;
    UI32_T      virtual_ip_up_count;
    VRRP_OM_Vrrp_Statistics_Info_T vrrp_statistic;
    L_SORT_LST_List_T assoc_ip_list;
} VRRP_OPER_ENTRY_T;

typedef struct VRRP_ASSOC_IP_ENTRY_S
{
    UI32_T      ifindex;
    UI8_T       vrid;
    UI8_T       assoc_ip_addr[4];
    UI32_T      row_status;
} VRRP_ASSOC_IP_ENTRY_T;

typedef struct VRRP_IP_PKT_HEADER_S
{
	UI8_T   verHlen;
	UI8_T   serviceType;
	UI16_T  totalLen;
	UI16_T  identification;
	UI16_T  flagFragOffset;
	UI8_T   ttl;
	UI8_T   protocol;
	UI16_T  headerChksum;
	UI8_T   srcIp[4];
	UI8_T   desIp[4];
} VRRP_IP_PKT_HEADER_T;

typedef struct VRRP_IP_PKT_S
{
	UI8_T   verHlen;
	UI8_T   serviceType;
	UI16_T  totalLen;
	UI16_T  identification;
	UI16_T  flagFragOffset;
	UI8_T   ttl;
	UI8_T   protocol;
	UI16_T  headerChksum;
	UI8_T   srcIp[4];
	UI8_T   desIp[4];
	I8_T    buffer[1040];
} VRRP_IP_PKT_T;

typedef struct VRRP_PKT_S
{
	UI8_T   verType;
	UI8_T   vrid;
	UI8_T   priority;
	UI8_T   countIpAddr;
	UI8_T   authType;
	UI8_T   adverInt;
	UI16_T  checksum;
} VRRP_PKT_T;

/* NAMING CONSTANT DECLARATION 
 */

/* VRRP IP Address Check */
#define     VRRP_TYPE_PRIMARY_IP_ADDRESS_INPUT      0x20000001
#define     VRRP_TYPE_SECONDARY_IP_ADDRESS_INPUT    0x20000002     

/* Status code of Engine */
#define     VRRP_TYPE_OK                            0x80000000
#define	    VRRP_TYPE_OPER_ENTRY_EXIST			    0x80000001
#define     VRRP_TYPE_OPER_ENTRY_NOT_EXIST          0x80000002
#define     VRRP_TYPE_ASSO_IP_ADDR_ENTRY_EXIST      0x80000003
#define     VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST  0x80000004
#define     VRRP_TYPE_OPER_IP_ADDR_COUNT_ZERO       0x80000005              
#define     VRRP_TYPE_ASSO_IP_ADDR_NOT_IN_SERVICE   0x80000006
#define     VRRP_TYPE_SET_PRIMARY_IP_FAIL          0x80000007  
#define     VRRP_TYPE_IP_ADDRESSES_NOT_VAILD        0x80000008
#define     VRRP_TYPE_INTERNAL_ERROR                0x80000009
#define     VRRP_TYPE_PARAMETER_ERROR               0x8000000A
#define     VRRP_TYPE_ACCESS_SUCCESS                0x8000000B
#define     VRRP_TYPE_MISCONFIGURATION              0x8000000C
#define     VRRP_TYPE_INTERFACE_NOT_EXIST           0x8000000D
#define     VRRP_TYPE_EXCESS_VRRP_NUMBER_ON_CHIP    0x8000000E
#define     VRRP_TYPE_SET_OWNER_PRIORITY            0x8000000F

/*  MREF control block declaration */
typedef struct	L_MREF_S
{
    UI32_T		cookie_for_driver;	/*	Multi-chip control buffer using	*/
    UI32_T		signature;			/*	MREF block signature	*/
    void        *desc;      /* descriptor of Data buffer managing block */
    char        *pdu;       /* client used processing pointer  */
    UI32_T      pdu_len;
    void		*peggy_back_func;	/* the last user func before destroy mref block */
    void		*cookie;			/* the parameter bring to peggy_back_func 		*/
#ifdef	L_MREF_DEBUG_ID
    UI32_T		my_id;				/*	the module, currently access this mref		*/
    UI32_T		next_id;			/*	next module will access this mref			*/
#endif
    UI32_T		access_time_stamp;	/*	last access time, MREF function accessed	*/
    void		*p2_ibd;			/*	point to associated IBD	*/
    UI16_T		lost_track;			/*	1--receive packet, 0-others	*/
    UI16_T		track_count;		/*	if count > 3, free this mref	*/
}   L_MREF_T, *L_MREF_P;


typedef struct VRRP_TASK_MSG_QUEUE_ITEM_S
{
    L_MREF_T    *pktFrtn;
    UI32_T      ipPktLen;
    UI32_T      rxRifNum;
    UI32_T      vid;
    UI8_T       src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T       dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T      port_no;
}   VRRP_TASK_MSG_QUEUE_ITEM_T;


#endif  /* VRRP_TYPES.h */

