/* MODULE NAME: mldsnp_type.H
* PURPOSE:
*   {1. What is covered in this file - function and scope}
*   {2. Related documents or hardware information}
* NOTES:
*   {Something must be known or noticed}
*   {1. How to use these functions - Give an example}
*   {2. Sequence of messages if applicable}
*   {3. Any design limitation}
*   {4. Any performance limitation}
*   {5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/03/2007    Macauley_Cheng Create
* Copyright(C)      Accton Corporation, 2007
*/

#ifndef _MLDSNP_TYPE_H
#define _MLDSNP_TYPE_H

/* INCLUDE FILE DECLARATIONS
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_type.h"
#include "sys_module.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "sys_time.h"
#include "l_hisam.h"
#include "l_sort_lst.h"
#include "l_stdlib.h"
#include "l_mm.h"
#include "l_math.h"
#include "l_prefix.h"

/* NAMING CONSTANT DECLARATIONS
*/
#define MLDSNP_TYPE_MAX_NUM_OF_LPORT      (   SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT   \
                                            * SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK  \
                                            + SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM)


/*Below define the mldsnp type value*/

typedef enum MLDSNP_TYPE_TraceId_E
{
    MLDSNP_TYPE_TRACE_FORWARD_GENERAL_QEURY,
    MLDSNP_TYPE_TRACE_FORWARD_GROUP_SPECIFIC_QEURY,
    MLDSNP_TYPE_TRACE_SEND_GENERAL_QUERY,
    MLDSNP_TYPE_TRACE_SEND_GENERAL_QUERY_PER_PORT,
    MLDSNP_TYPE_TRACE_SEND_GROUP_SPECIFIC_QUERY,
    MLDSNP_TYPE_TRACE_FORWARD_V1_REPORT,
    MLDSNP_TYPE_TRACE_FORWARD_V2_REPORT,
    MLDSNP_TYPE_TRACE_FLOOD_V1_REPORT,
    MLDSNP_TYPE_TRACE_SEND_REPORT,
    MLDSNP_TYPE_TRACE_SEND_V2REPORT,
    MLDSNP_TYPE_TRACE_FORWARD_DONE,
    MLDSNP_TYPE_TRACE_SEND_DONE,
    MLDSNP_TYPE_TRACE_UNKNOWN_PDU,
    MLDSNP_TYPE_TRACE_MRD_SOLICITATION,
    MLDSNP_TYPE_TRACE_MRD_ADVERTISEMENT,
    MLDSNP_TYPE_TRACE_CREATE_TIMER,
    MLDSNP_TYPE_TRACE_CREATE_TIMER_PARA,
    MLDSNP_TYPE_TRACE_CREATE_TIMER_SRCLIST,
    MLDSNP_TYPE_TRACE_HOST_IP,
    MLDSNP_TYPE_TRACE_GRP_SRC_NODE,
    MLDSNP_TYPE_TRACE_WELLKNOWN_PDU, /*zhimin, 20160622*/
}MLDSNP_TYPE_TraceId_T;

/*defien the packet field length*/
#define MLDSNP_TYPE_ETHER_MAC_LEN           4  /*the length address of group ip will map to ethernet mac*/
#define MLDSNP_TYPE_HOP_BY_HOP_LEN          8
#define MLDSNP_TYPE_IPV6_HEADER_LEN         40
#define MLDSNP_TYPE_IPV6_SRC_IP_LEN         SYS_ADPT_IPV6_ADDR_LEN
#define MLDSNP_TYPE_IPV6_DST_IP_LEN         SYS_ADPT_IPV6_ADDR_LEN
#define MLDSNP_TYPE_ICMPV6_COMMON_LEN       24
#define MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC 10  /*define only process the max src ip in each record*/

#define MLDSNP_TYPE_QUERY_V1_LEN            24
#define MLDSNP_TYPE_QUERY_V2_LEN            28
#define MLDSNP_TYPE_MRD_SOLICITAION_LEN     4
#define MLDSNP_TYPE_MRD_ADVERTISEMENT_LEN   8
#define MLDSNP_TYPE_REPORT_V1_LEN           24
#define MLDSNP_TYPE_REPORT_V2_LEN           8

#define MLDSNP_TYPE_ICMPV6_GROUP_ADDR_LEN   MLDSNP_TYPE_IPV6_DST_IP_LEN
#define MLDSNP_TYPE_IPV6_ETH_TYPE           0x86DD
#define MLDSNP_TYPE_IPV6_HOP_BY_HOP_HEAD    0x00
#define MLDSNP_TYPE_IPV6_ICMPV6_HAED        58
#define MLDSNP_TYPE_IPV6_TCP_HEAD           6
#define MLDSNP_TYPE_IPV6_UDP_HEAD           17
#define MLDSNP_TYPE_IPV6_OTHER_HEAD         59
#define MLDSNP_TYPE_IPV6_PIM_HEAD         103

#define MLDSNP_TYPE_MAX_PAYLOAD_SIZE        120
#define MLDSNP_TYPE_MAX_ALOCATE_FRAME_SIZE   1450 /*we allow over payload size by shall limit by frame size, for easy implement*/

#define MLDSNP_TYPE_PIM_HEAD_SIZE                 4
#define MLDSNP_TYPE_PIM_HELLO_HOLDTIME_OPTION     1
#define MLDSNP_TYPE_PIM_HELLO                     0x20

/* key index for HISAM table
 */
typedef enum MLDSNP_TYPE_HisamKeyIndex_E
{
    MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC =0                 /*VID, GROUP, SRC*/
}MLDSNP_TYPE_HisamKeyIndex_T;

typedef enum MLDSNP_TYPE_UnknownBehavior_E
{
    MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD          = VAL_mldSnoopUnknownMcastMode_flood,
    MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT = VAL_mldSnoopUnknownMcastMode_toRouterPort,
    MLDSNP_TYPE_UNKNOWN_BEHAVIOR_DROP           = VAL_mldSnoopUnknownMcastMode_drop
}MLDSNP_TYPE_UnknownBehavior_T;

typedef enum MLDSNP_TYPE_JoinType_E
{
    MLDSNP_TYPE_JOIN_UNKNOWN,
    MLDSNP_TYPE_JOIN_STATIC,
    MLDSNP_TYPE_JOIN_DYNAMIC,
}MLDSNP_TYPE_JoinType_T;

typedef enum MLDSNP_TYPE_ListType_E
{
    MLDSNP_TYPE_LIST_INCLUDE,
    MLDSNP_TYPE_LIST_REQUEST,
    MLDSNP_TYPE_LIST_EXCLUDE
}MLDSNP_TYPE_ListType_T;

typedef enum MLDSNP_TYPE_ReturnValue_E
{
    MLDSNP_TYPE_RETURN_FAIL=0,
    MLDSNP_TYPE_RETURN_SUCCESS
}MLDSNP_TYPE_ReturnValue_T;

typedef enum MLDSNP_TYPE_Message_E
{
    MLDSNP_TYPE_QUERY               =130,
    MLDSNP_TYPE_REPORT_V1           =131,
    MLDSNP_TYPE_REPORT_V2           =143,
    MLDSNP_TYPE_DONE                =132,
    MLDSNP_TYPE_MRD_ADVERTISEMENT   =151,
    MLDSNP_TYPE_MRD_SOLICITATION    =152,
    MLDSNP_TYPE_MRD_TERMINATION     =153,
    MLDSNP_TYPE_DATA
}MLDSNP_TYPE_Message_T;

typedef enum MLDSNP_TYPE_QuerierStatus_E
{
    MLDSNP_TYPE_QUERIER_ENABLED  =VAL_mldSnoopQuerier_enabled,
    MLDSNP_TYPE_QUERIER_DISABLED =VAL_mldSnoopQuerier_disabled
}MLDSNP_TYPE_QuerierStatus_T;

typedef enum MLDSNP_TYPE_MLDSNP_STATUS_E
{
    MLDSNP_TYPE_MLDSNP_ENABLED  =VAL_mldSnoopStatus_enabled,
	MLDSNP_TYPE_MLDSNP_DISABLED =VAL_mldSnoopStatus_disabled
}MLDSNP_TYPE_MLDSNP_STATUS_T;

typedef enum MLDSNP_TYPE_ImmediateStatus_E
{
    MLDSNP_TYPE_IMMEDIATE_ENABLED  =VAL_mldSnoopCurrentVlanImmediateLeave_enabled,
    MLDSNP_TYPE_IMMEDIATE_DISABLED =VAL_mldSnoopCurrentVlanImmediateLeave_disabled
}MLDSNP_TYPE_ImmediateStatus_T;

typedef enum MLDSNP_TYPE_ImmediateByHostStatus_E
{
    MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED  =VAL_mldSnoopCurrentVlanImmediateLeaveByHost_enabled,
    MLDSNP_TYPE_IMMEDIATE_BYHOST_DISABLED =VAL_mldSnoopCurrentVlanImmediateLeaveByHost_disabled
}MLDSNP_TYPE_ImmediateByHostStatus_T;

typedef enum MLDSNP_TYPE_RecordType_E
{
    MLDSNP_TYPE_IS_INCLUDE_MODE=1,
    MLDSNP_TYPE_IS_EXCLUDE_MODE,
    MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE,
    MLDSNP_TYPE_CHANGE_TO_EXCLUDE_MODE,
    MLDSNP_TYPE_ALLOW_NEW_SOURCES,
    MLDSNP_TYPE_BLOCK_OLD_SOURCES
}MLDSNP_TYPE_RecordType_T, MLDSNP_TYPE_CurrentMode_T;

typedef enum MLDSNP_TYPE_Version_E
{
    MLDSNP_TYPE_VERSION_1=MIN_mldSnoopVersion,
    MLDSNP_TYPE_VERSION_2=MAX_mldSnoopVersion,
}MLDSNP_TYPE_Version_T;

typedef enum MLDSNP_TYPE_ProxyReporting_E
{
    MLDSNP_TYPE_PROXY_REPORTING_ENABLE  = VAL_mldSnoopProxyReporting_enabled,
    MLDSNP_TYPE_PROXY_REPORTING_DISABLE = VAL_mldSnoopProxyReporting_disabled
}MLDSNP_TYPE_ProxyReporting_T;

typedef enum MLDSNP_TYPE_TxReason_E
{
    MLDSNP_TYPE_V1_REPORT,
    MLDSNP_TYPE_V2_REPORT,
    MLDSNP_TYPE_V1_LEAVE,
    MLDSNP_TYPE_V1_G_QUERY,
    MLDSNP_TYPE_V2_G_QUERY,
    MLDSNP_TYPE_V1_G_S_QUERY,
    MLDSNP_TYPE_V2_G_S_QUERY,
    MLDSNP_TYPE_FORWARD,
}MLDSNP_TYPE_TxReason_T;

typedef enum MLDSNP_TYPE_Statistics_E
{
    MLDSNP_TYPE_STAT_GRROUPS,
    MLDSNP_TYPE_STAT_JOIN_SEND,
    MLDSNP_TYPE_STAT_JOINS,
    MLDSNP_TYPE_STAT_JOIN_SUCC,
    MLDSNP_TYPE_STAT_LEAVE_SEND,
    MLDSNP_TYPE_STAT_LEAVE,
    MLDSNP_TYPE_STAT_GQ_SEND,
    MLDSNP_TYPE_STAT_GQ_RCVD,
    MLDSNP_TYPE_STAT_SQ_SEND,
    MLDSNP_TYPE_STAT_SQ_RCVD,
    MLDSNP_TYPE_STAT_INVALID,
    MLDSNP_TYPE_STAT_DROP_FILTER,
    MLDSNP_TYPE_STAT_DROP_MROUTER,
    MLDSNP_TYPE_STAT_DROP_RATE_LIMIT,
}MLDSNP_TYPE_Statistics_T;

typedef enum MLDSNP_TYPE_ProcessResult_E
{
    MLDSNP_TYPE_PROC_FAIL = 0,
    MLDSNP_TYPE_PROC_SUCC   ,
    MLDSNP_TYPE_PROC_FLOOD,
}MLDSNP_TYPE_ProcessResult_T;

/*define the default value*/
#define MLDSNP_TYPE_DFLT_MLDSNP_STATUS                SYS_DFLT_MLDSNP_STATUS//VAL_mldSnoopStatus_disabled
#define MLDSNP_TYPE_DFLT_VER                          SYS_DFLT_MLDSNP_VERSION//MAX_mldSnoopVersion //2
#define MLDSNP_TYPE_DFLT_QUERIER_STATUS               SYS_DFLT_MLDSNP_QUERIER_STATUS//VAL_mldSnoopQuerier_disabled
#define MLDSNP_TYEP_DFLT_ROBUST_VALUE                 SYS_DFLT_MLDSNP_ROBUST_VALUE//2
#define MLDSNP_TYPE_DFLT_QUERY_INTERVAL               SYS_DFLT_MLSDNP_QUERY_INTERVAL//125 /*sec.*/
#define MLDSNP_TYPE_DFLT_MAX_RESP_INTERVAL            10  /*sec.*/
#define MLDSNP_TYPE_DFLT_ROUTER_EXP_TIME              300 /*sec.*/
#define MLDSNP_TYPE_DFLT_UNKNOWN_TIMEOUT              MLDSNP_TYPE_DFLT_ROUTER_EXP_TIME/*sec, define the unknown stream how long will timeout*/
#define MLDSNP_TYPE_DFLT_FLOOD_BEHAVIOR               SYS_DFLT_MLDSNP_UNKNOWN_MULTICAST_MOD//VAL_mldSnoopUnknownMcastMode_toRouterPort
#define MLDSNP_TYPE_DFLT_IMMEDIATE_STATUS             SYS_DFLT_MLDSNP_IMMEIDATE_STATUS//VAL_mldSnoopCurrentVlanImmediateLeave_disabled
#define MLDSNP_TYPE_DFLT_IMMEDIATE_BYHOST_STATUS      SYS_DFLT_MLDSNP_IMMEIDATE_BYHOST_STATUS//VAL_mldSnoopCurrentVlanImmediateLeaveByHost_disabled
#define MLDSNP_TYPE_DFLT_LAST_LISTENER_QUERY_INTERVAL 1 /*sec.*/
#define MLDSNP_TYPE_DFLT_UNSOLICITED_REPORT_INTERVAL  SYS_DFLT_MLDSNP_UNSOLICIT_REPORT_INTERVAL /*sec.*/
#define MLDSNP_TYPE_DFLT_SOLICITATION_INTERVAL   MLDSNP_TYPE_DFLT_ROUTER_EXP_TIME/2

/*defin min and max vlaue, this value should be replaced by mib define
  the value now is defined un-normal*/
#define MLDSNP_TYPE_MAX_ROBUSTNESS                 MAX_mldSnoopRobustness //10
#define MLDSNP_TYPE_MIN_ROBUSTNESS                 MIN_mldSnoopRobustness //2
#define MLDSNP_TYPE_MAX_QUERY_INTERVAL             MAX_mldSnoopQueryInterval //125
#define MLDSNP_TYPE_MIN_QUERY_INTERVAL             MIN_mldSnoopQueryInterval //60
#define MLDSNP_TYPE_MAX_RESPONSE_INTERVAL          MAX_mldSnoopQueryMaxResponseTime //25
#define MLDSNP_TYPE_MIN_RESPONSE_INTERVAL          MIN_mldSnoopQueryMaxResponseTime //5
#define MLDSNP_TYPE_MAX_ROUTER_EXPIRE_TIME         MAX_mldSnoopRouterPortExpireTime //500
#define MLDSNP_TYPE_MIN_ROUTER_EXPIRE_TIME         MIN_mldSnoopRouterPortExpireTime //300


/* MACRO FUNCTION DECLARATIONS
*/
#define MLDSNP_TYPE_ipv6ToMac(__gip_p, __mac_ap) \
{  \
     UI16_T  mac_start = MLDSNP_TYPE_IPV6_DST_IP_LEN - SYS_ADPT_MAC_ADDR_LEN+2; \
     __mac_ap[0] = 0x33; \
     __mac_ap[1] = 0x33; \
     memcpy(&__mac_ap[2], &__gip_p[mac_start], 4); \
}
#define MLDSNP_TYPE_MacToipv6(__gip_p, __mac_ap) \
{  \
    UI16_T  mac_start = MLDSNP_TYPE_IPV6_DST_IP_LEN - SYS_ADPT_MAC_ADDR_LEN+2; \
    __gip_p[0]=0xff; \
    __gip_p[1]=0x02; \
    memcpy(&__gip_p[mac_start], &__mac_ap[2], 4); \
}
#define MLDSNP_TYPE_AddPortIntoPortBitMap(__port_no, __port_bitmap)  __port_bitmap[(__port_no-1)/8] |= ( 0x80 >> (__port_no-1)%8 )

#define MLDSNP_TYPE_DeletePortFromPorBitMap(__port_no, __port_bitmap)  __port_bitmap[(__port_no-1)/8] &= ~( 0x80 >> (__port_no-1)%8 )

#define MLDSNP_TYPE_IsPortInPortBitMap(__port_no, __port_bitmap) ((__port_bitmap[(__port_no-1)/8] &( 0x80 >> (__port_no-1)%8 ))?TRUE:FALSE)

#define MLDSNP_TYPE_CHECK_FLAG(V,F)      ((V) & (F))
#define MLDSNP_TYPE_SET_FLAG(V,F)        (V) = (V) | (F)
#define MLDSNP_TYPE_UNSET_FLAG(V,F)      (V) = (V) & ~(F)
#define MLDSNP_TYPE_FLAG_ISSET(V,F)      (((V) & (F)) == (F))

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
#endif/* End of mldsnp_type_H */


