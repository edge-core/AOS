/* MODULE NAME:  rip_type.h
 * PURPOSE:
 *     Define common types used in RIP.
 *
 * NOTES:
 *
 * HISTORY
 *    05/12/2008 - Lin.Li, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef RIP_TYPE_H
#define RIP_TYPE_H

#include "sys_type.h"
#include "sys_adpt.h"
     

/**********************************
** definitions for return value **
**********************************
*/
enum
{
    RIP_TYPE_RESULT_OK = 0,
    RIP_TYPE_RESULT_FAIL,
    RIP_TYPE_RESULT_INVALID_COMMAND,
    RIP_TYPE_RESULT_INVALID_ARG,
    RIP_TYPE_RESULT_SEND_MSG_FAIL,
    RIP_TYPE_RESULT_VR_NOT_EXIST,
    RIP_TYPE_RESULT_INTERFACE_NOT_EXIST,
    RIP_TYPE_RESULT_INSTANCE_NOT_EXIST,
    RIP_TYPE_RESULT_PROCESS_NOT_EXIST,
    RIP_TYPE_RESULT_UNKNOWN_ERR,
    RIP_TYPE_RESULT_INVALID_VALUE,
    RIP_TYPE_RESULT_INTERFACE_EXIST,
    RIP_TYPE_RESULT_VERSION_INVALID,
    RIP_TYPE_RESULT_SPLIT_HORIZON_INVALID,
    RIP_TYPE_RESULT_ADDRESS_INVALID,
    RIP_TYPE_RESULT_PREFIX_INVALID,
    RIP_TYPE_RESULT_NETWORK_EXIST,
    RIP_TYPE_RESULT_NETWORK_NOT_EXIST,
    RIP_TYPE_RESULT_TIMER_VALUE_INVALID,
    RIP_TYPE_RESULT_CANT_CHANGE_BUFFER_SIZE,
    RIP_TYPE_RESULT_METRIC_INVALID,
    RIP_TYPE_RESULT_DISTANCE_INVALID,
    RIP_TYPE_RESULT_DISTANCE_EXIST,
    RIP_TYPE_RESULT_AUTH_TYPE_INVALID,
    RIP_TYPE_RESULT_AUTH_LENGTH_INVALID,
    RIP_TYPE_RESULT_AUTH_STR_EXIST,
    RIP_TYPE_RESULT_ROUTE_NOT_EXIST
};

typedef enum RIP_TYPE_Packet_Debug_Type_E
{
    RIP_TYPE_PACKET_DEBUG_TYPE = 1,
    RIP_TYPE_PACKET_DEBUG_TYPE_SEND,
    RIP_TYPE_PACKET_DEBUG_TYPE_SENDANDDETAIL,
    RIP_TYPE_PACKET_DEBUG_TYPE_RECEIVE,
    RIP_TYPE_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL,
    RIP_TYPE_PACKET_DEBUG_TYPE_DETAIL
}RIP_TYPE_Packet_Debug_Type_T;
    
typedef struct RIP_TYPE_Timer_S
{
    UI32_T update;
    UI32_T timeout;
    UI32_T carbage_collection;
} RIP_TYPE_Timer_T;

enum RIP_TYPE_Version_E
{
    RIP_TYPE_VERSION_UNSPEC = 0,
    RIP_TYPE_VERSION_VERSION_1,
    RIP_TYPE_VERSION_VERSION_2,
    RIP_TYPE_VERSION_VERSION_1_AND_2,
    RIP_TYPE_VERSION_VERSION_1_COMPATIBLE,
    RIP_TYPE_VERSION_VERSION_MAX
};

enum RIP_TYPE_Auth_Mode_E
{
    RIP_TYPE_NO_AUTH = 0,
    RIP_TYPE_AUTH_DATA,
    RIP_TYPE_AUTH_SIMPLE_PASSWORD,
    RIP_TYPE_AUTH_MD5
};

enum RIP_TYPE_Split_Horizon_E
{
    RIP_TYPE_SPLIT_HORIZON_POISONED = 0,
    RIP_TYPE_SPLIT_HORIZON_NONE,
    RIP_TYPE_SPLIT_HORIZON
};

enum RIP_TYPE_Global_Version_E
{
    RIP_TYPE_GLOBAL_VERSION_1 = 1,
    RIP_TYPE_GLOBAL_VERSION_2,
    RIP_TYPE_GLOBAL_VERSION_BY_INTERFACE
};

enum RIP_TYPE_Distribute_Type_E
{
  RIP_TYPE_DISTRIBUTE_IN = 1,
  RIP_TYPE_DISTRIBUTE_OUT,
  RIP_TYPE_DISTRIBUTE_MAX
};

enum RIP_TYPE_Distribute_List_Type_E
{
    RIP_TYPE_DISTRIBUTE_ACCESS_LIST = 1,
    RIP_TYPE_DISTRIBUTE_PREFIX_LIST = 2
};
typedef struct RIP_TYPE_Route_Entry_S
{
    UI32_T dest_pfxlen;
    UI32_T dest_addr;
    UI32_T nexthop_addr;
    UI32_T from_addr;
    UI32_T metric;
    UI32_T ifindex;
    UI32_T distance;
    int    type;
    int    sub_type;
    char   type_str[3];
    char   timebuf [100];
} RIP_TYPE_Route_Entry_T;

typedef struct RIP_TYPE_Peer_Entry_S
{
    UI32_T peer_addr;
    int    domain;
    char   timebuf[25];
    long   uptime;
    int    ifindex;
    int    version;
    int    recv_badpackets;
    int    recv_badroutes;
} RIP_TYPE_Peer_Entry_T;

typedef struct RIP_TYPE_Global_Statistics_S
{
    int global_route_changes;
    int global_queries;
} RIP_TYPE_Global_Statistics_T;

/* RIP Statistics DATA Structure */
typedef struct Rip2IfStatEntry_S {
    /* The IP Address of this system on the indicated subnet. */
    UI32_T  rip2_if_stat_address;

    /* The number of RIP response packets received by the  RIP  process
       which were subsequently discarded for any reason. */
    UI32_T  rip2_if_stat_rcv_bad_packets;

    /* The number of routes, in  valid  RIP  packets,
       which were ignored for any reason. */
    UI32_T  rip2_if_stat_rcv_bad_routes;

    /* The number of triggered RIP updates sent on this interface. */
    UI32_T  rip2_if_stat_sent_updates;

    /* Writing invalid has  the  effect  of  deleting this interface. */
    BOOL_T  rip2_if_stat_status;

} Rip2IfStatEntry_T;
typedef struct Rip2IfConfEntry_S {
    /* The IP Address of this system on the indicated subnet. */
    UI32_T  rip2_if_conf_address;

    /* Value inserted into the Routing Domain field of all RIP packets
       sent on this interface. */
    /* read-write */
    UI8_T  rip2_if_conf_domain[2];      /* always 0 */

    /* The type of Authentication used on this interface. */
    /* Value : noAuthentication, simplePassword */
    /* Default : noAuthentication */
    /* read-write */
    UI32_T  rip2_if_conf_auth_type;

    /* read-write */
    char rip2_if_conf_auth_key[17];

    /* Value : 
        doNotSend, ripVersion1, rip1Compatible, ripVersion2
       Default : rip1Compatible 
    */
    /* read-write */
    UI32_T  rip2_if_conf_send;

    /* Value : 
        rip1, rip2, rip1OrRip2
       Default : rip1OrRip2
    */
    /* read-write */
    UI32_T  rip2_if_conf_receive;

    /* Value : INTEGER (0 - 15) */
    /* read-write */
    UI32_T  rip2_if_conf_default_metric;

    UI32_T  rip2_if_conf_status;

    UI32_T  rip2_if_conf_poison;

    UI32_T  rip2_if_conf_src_address;
} Rip2IfConfEntry_T;


#endif    /* End of RIP_TYPE_H */

