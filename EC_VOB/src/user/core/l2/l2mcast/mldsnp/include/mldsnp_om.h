/* MODULE NAME: mldsnp_om.H
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


#ifndef _MLDSNP_OM_H
#define _MLDSNP_OM_H

/* INCLUDE FILE DECLARATIONS
*/
#include "mldsnp_type.h"
#include "mldsnp_timer.h"
#include "l_linklist.h"

/* NAMING CONSTANT DECLARATIONS
*/

enum MLDSNP_OM_IPCMD_Command_E
{
    MLDSNP_OM_IPCCMD_GETGLOBALCONF,
    MLDSNP_OM_IPCCMD_GETGROUPPORTLIST,
    MLDSNP_OM_IPCCMD_GETHISAMENTRYINFO,
    MLDSNP_OM_IPCCMD_GETIMMEDIATELEAVESTATUS,
    MLDSNP_OM_IPCCMD_GETIMMEDIATELEAVEBYHOSTSTATUS,    
    MLDSNP_OM_IPCCMD_GETNEXTIMMEDIATELEAVESTATUS,
    MLDSNP_OM_IPCCMD_GETNEXTIMMEDIATELEAVEBYHOSTSTATUS,    
    MLDSNP_OM_IPCCMD_GETLASTLISTENERQUERYINTERVAL,
    MLDSNP_OM_IPCCMD_GETLISTENERINTERVAL,
    MLDSNP_OM_IPCCMD_GETMLDSNPVER,
    MLDSNP_OM_IPCCMD_GETMLDSTATUS,
    MLDSNP_OM_IPCCMD_GETNEXTGROUPPORTLIST,
    MLDSNP_OM_IPCCMD_GETNEXTRUNNINGPORTJOINSTATICGROUP,
    MLDSNP_OM_IPCCMD_GETNEXTPORTJOINSTATICGROUP,
    MLDSNP_OM_IPCCMD_GETSTATICGROUPPORTLIST,
    MLDSNP_OM_IPCCMD_GETNEXTSTATICGROUPPORTLIST,
    MLDSNP_OM_IPCCMD_GETOLDVERQUERIERPRESENTTIMEOUT,
    MLDSNP_OM_IPCCMD_GETOTHERQUERYPRESENTINTERVAL,
    MLDSNP_OM_IPCCMD_GETQUERIERRUNNINGSTATUS,
    MLDSNP_OM_IPCCMD_GETQUERIERSTATUS,
    MLDSNP_OM_IPCCMD_GETQUERYINTERVAL,
    MLDSNP_OM_IPCCMD_GETQUERYRESPONSEINTERVAL,
    MLDSNP_OM_IPCCMD_GETROBUSTNESSVALUE,
    MLDSNP_OM_IPCCMD_GETROUTEREXPIRETIME,
    MLDSNP_OM_IPCCMD_GETRUNNINGGLOBALCONF,
    MLDSNP_OM_IPCCMD_GETRUNNINGIMMEDIATELEAVESTATUS,
    MLDSNP_OM_IPCCMD_GETRUNNINGROUTERPORTLIST,
    MLDSNP_OM_IPCCMD_GETUNKNOWNFLOODBEHAVIOR,
    MLDSNP_OM_IPCCMD_GETRUNNINGUNKNOWNFLOODBEHAVIOR,
    MLDSNP_OM_IPCCMD_GETUNSOLICITEDREPORTINTERVAL,
    MLDSNP_OM_IPCCMD_GETVLANROUTERPORTLIST,
    MLDSNP_OM_IPCCMD_GETNEXTVLANROUTERPORTLIST,
    MLDSNP_OM_IPCCMD_GETVLANROUTERPORTEXPIRE,
    MLDSNP_OM_IPCCMD_GETNEXTPORTGROUPSOURCELIST,
    MLDSNP_OM_IPCCMD_GETPORTGROUPSOURCELIST,
    MLDSNP_OM_IPCCMD_GETNEXTPORTGROUPSOURCE,
    MLDSNP_OM_IPCCMD_GETNEXTPORTGROUPSOURCEHOST,
    MLDSNP_OM_IPCCMD_GET_MLD_FILTER_STATUS,
    MLDSNP_OM_IPCCMD_GET_MLD_PRIFILE_MODE,
    MLDSNP_OM_IPCCMD_GET_MLD_PRIFILE_PORT,
    MLDSNP_OM_IPCCMD_GETNEXT_MLD_PRIFILE_ID,
    MLDSNP_OM_IPCCMD_GETNEXT_MLD_PRIFILE_GROUP,
    MLDSNP_OM_IPCCMD_GET_MLD_THROTTLE_INFO,
    MLDSNP_OM_IPCCMD_EXIST_MLD_CREATE_PRIFILE,
    MLDSNP_OM_IPCCMD_GET_MLD_REPORT_LIMIT_PER_SECOND,
    MLDSNP_OM_IPCCMD_GET_NEXT_MLD_REPORT_LIMIT_PER_SECOND,
    MLDSNP_OM_IPCCMD_GET_QUERY_GUARD_STATUS,
    MLDSNP_OM_IPCCMD_GETNEXT_QUERY_GUARD_STATUS,
    MLDSNP_OM_IPCCMD_GET_RUNNING_QUERY_GUARD_STATUS,
    MLDSNP_OM_IPCCMD_GET_MULTICAST_DATA_DROP_STATUS,
    MLDSNP_OM_IPCCMD_GETNEXT_MULTICAST_DATA_DROP_STATUS,
    MLDSNP_OM_IPCCMD_GET_RUNNING_MULTICAST_DATA_DROP_STATUS,
    MLDSNP_OM_IPCCMD_GET_PROXY_REPORTING,
    MLDSNP_OM_IPCCMD_GET_TOTAL_ENTRY,
    MLDSNP_OM_IPCCMD_GETRUNNINGVLANCFG,
    MLDSNP_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC       /*an flag to say up define need retunrn value, but below define needn't*/
};

#define MLDSNP_OM_MSGBUF_TYPE_SIZE    sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->type)


#define MLDSNP_OM_GET_MSGBUFSIZE(type_name) \
    ((uintptr_t)(&((MLDSNP_OM_IPCMsg_T *)0)->data) + sizeof(type_name))


#define MLDSNP_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    ((uintptr_t)(&((MLDSNP_OM_IPCMsg_T *)0)->type))


#define MLDSNP_OM_MSG_CMD(msg_p)    (((MLDSNP_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define MLDSNP_OM_MSG_RETVAL(msg_p) (((MLDSNP_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.result_bool)

#define MLDSNP_OM_MSG_DATA(msg_p)   ((void *)&((MLDSNP_OM_IPCMsg_T *)(msg_p)->msg_buf)->data)

/* These value will be use by om handler to set msg.type.result
 *   MLDSNP_MGR_IPC_RESULT_OK   - only use when API has no return value
 *                                 and mgr deal this request.
 *   MLDSNP_MGR_IPC_RESULT_FAIL - it denote that mgr handler can't deal
 *                                 the request. (ex. in transition mode)
 */
#define MLDSNP_OM_IPC_RESULT_OK    (0)
#define MLDSNP_OM_IPC_RESULT_FAIL  (-1)

typedef struct MDLSNP_OM_Counter_S
{
    /*Group records*/
    UI32_T num_grecs;

    /*Send group reports*/
    UI32_T num_joins_send;

    /*Group reports */
    UI32_T num_joins;

    /*Succeed group reports*/
    UI32_T num_joins_succ;

    /*Send group leave*/
    UI32_T num_leaves_send;

    /*Group leave*/
    UI32_T num_leaves;

    /*Send general query*/
    UI32_T num_gq_send;

    /*Receive general query*/
    UI32_T num_gq_recv;

    /*Send specific query*/
    UI32_T num_sq_send;

    /*Receive specific query*/
    UI32_T num_sq_recv;

    /*Receive invalid mld message*/
    UI32_T num_invalid_mld_recv;

    #if(SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    /*dropped by igmp filter*/
    UI32_T num_drop_by_filter;
    #endif

    /*dropped by received on mvr source or mrouter port*/
    UI32_T num_drop_by_mroute_port;

    #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
    /*dropped by rate limit*/
    UI32_T num_drop_by_rate_limit;
    #endif
}MLDSNP_OM_Counter_T;


typedef struct MLDSNP_OM_InfStat_S
{
    UI32_T              active_group_cnt;
    UI32_T              query_uptime;    
    UI32_T              query_exptime;
    UI32_T              other_query_uptime;
    UI32_T              other_query_exptime;
    UI32_T              unsolicit_exptime;
    UI8_T               querier_ip_addr[SYS_ADPT_IPV6_ADDR_LEN];
    MLDSNP_OM_Counter_T counter;
} MLDSNP_OM_InfStat_T;



/*store the UI configuration
  */
typedef struct MLDSNP_OM_Cfg_S
{

    MLDSNP_TYPE_MLDSNP_STATUS_T    mldsnp_status;
    MLDSNP_TYPE_QuerierStatus_T    querier_status;
    #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    MLDSNP_TYPE_UnknownBehavior_T  unknown_flood_behavior;
    #else
    MLDSNP_TYPE_UnknownBehavior_T  unknown_flood_behavior[SYS_ADPT_MAX_VLAN_ID];
    #endif
    MLDSNP_TYPE_Version_T          version;                      /* Version 1/2 */

    UI16_T                         robust_value;                 /*7.1, 7.9, 9.1, 9.9*/
    UI16_T                         query_interval;               /*7.2, 9.2*/
    UI16_T                         query_response_interval;      /*7.3, 9.3*/
    UI16_T                         listener_interval;            /*7.4, 9.4,robust * query_interval*/
    UI16_T                         last_listner_query_interval;	 /*7.8, 9.8*/
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T   proxy_reporting;
    UI16_T                         unsolicited_report_interval;	 /*7.9, 9.11*/
#endif
    UI16_T                         querier_start_sent_count;     /*this variable record the startup query sent counts*/

    UI16_T                         router_exp_time;

    BOOL_T                         flag;
    #define                        MLDSNP_OM_MROUTE_ENABLED  BIT_1

#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE || SYS_CPNT_IPV6_MULTICAST_DATA_DROP == TRUE)
    UI8_T conf[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
#define MLDSNP_OM_QUERY_DROP_ENABLED           BIT_0
#define MLDSNP_OM_MULTICAST_DATA_DROP_ENABLED  BIT_1

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)
    UI8_T port_mld_pkt_lmt_conf[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    UI8_T port_mld_pkt_lmt_oper[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN== TRUE)
    UI16_T vlan_mld_pkt_lmt_conf[SYS_ADPT_MAX_VLAN_ID];
    UI16_T vlan_mld_pkt_lmt_oper[SYS_ADPT_MAX_VLAN_ID];
#endif

} MLDSNP_OM_Cfg_T;

/*struct for getting runing configuration
  */
typedef struct MLDSNP_OM_RunningCfg_S
{
    MLDSNP_TYPE_MLDSNP_STATUS_T    mldsnp_status;
    MLDSNP_TYPE_QuerierStatus_T    querier_status;
    MLDSNP_TYPE_UnknownBehavior_T  unknown_flood_behavior;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T   proxy_reporting;
    UI32_T                         unsolicit_report_interval;
#endif
    UI16_T                         version;                           /* Version 1/2 */
    UI16_T                         robust_value;                      /*7.1, 7.9, 9.1, 9.9*/
    UI16_T                         query_interval;                    /*7.2, 9.2*/
    UI16_T                         query_response_interval;           /*7.3, 9.3*/
    UI16_T                         router_exp_time;


    BOOL_T                         mldsnp_status_changed;
    BOOL_T                         version_changed;
    BOOL_T                         querier_status_changed;
    BOOL_T                         query_interval_changed;
    BOOL_T                         query_response_interval_changed;
    BOOL_T                         robust_value_changed;
    BOOL_T                         router_exp_time_changed;
    BOOL_T                         unknown_flood_behaviro_changed;
}MLDSNP_OM_RunningCfg_T;

typedef struct MLDSNP_OM_VlanRunningCfg_S
{
    UI8_T router_port_bitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == TRUE)
    MLDSNP_TYPE_UnknownBehavior_T flood_behavior;
    #endif
    MLDSNP_TYPE_ImmediateStatus_T  immediate_leave_status;
    MLDSNP_TYPE_ImmediateByHostStatus_T  immediate_leave_byhost_status;
    #if(SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
    UI32_T pkt_ratelimit;
    #endif
}MLDSNP_OM_VlanRunningCfg_T;
/*struct for UI to get group entry portlist
  */
typedef struct MLDSNP_OM_GroupInfo_S
{
    UI16_T  vid;
    UI8_T   gip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN];          /*group ipv6*/
    UI8_T   sip_a[MLDSNP_TYPE_IPV6_DST_IP_LEN];          /*source ipv6*/

    /* 0 -> not join this group
     * 1 -> join this gorup
     */
    UI8_T   dynamic_port_bitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    /* 0 -> not join this group
     * 1 -> join this gorup
     */
    UI8_T   static_port_bitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    /* 0 -> not join this group
     * 1 -> join this gorup
     */
    UI8_T   unknown_port_bitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
} MLDSNP_OM_GroupInfo_T;

/*struct for UI to get the port jion source list
  */
typedef struct MLDSNP_OM_PortSourceListInfo_S
{
    UI16_T                    vid;
    UI16_T                    port;
    UI16_T                    num_of_req;
    UI16_T                    num_of_ex;
    MLDSNP_TYPE_JoinType_T    join_type;
    MLDSNP_TYPE_CurrentMode_T cur_mode;
    UI32_T                    filter_time_elapse;
    UI8_T                     gip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
    UI8_T                     request_list[MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC][MLDSNP_TYPE_IPV6_SRC_IP_LEN];
    UI8_T                     exclude_list[MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC][MLDSNP_TYPE_IPV6_SRC_IP_LEN];

}MLDSNP_OM_PortSourceListInfo_T;

typedef struct MLDSNP_OM_PortSourceInfo_S
{
    UI16_T                    vid;
    UI16_T                    port;
    MLDSNP_TYPE_JoinType_T    join_type;
    UI32_T                    expire;
    UI32_T                    up_time;
    UI32_T                    unreply_q_count;
    UI8_T                     gip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
    UI8_T                     sip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
}MLDSNP_OM_PortSourceInfo_T;

typedef struct MLDSNP_OM_PortHostInfo_S
{
    UI16_T                    vid;
    UI16_T                    port;
    UI8_T                     gip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
    UI8_T                     sip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
    UI8_T                     host_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
}MLDSNP_OM_PortHostInfo_T;

/*struct for UI to get the router port list*/
 typedef struct MLDSNP_OM_RouterPortList_S
 {
     /* key - to specify a unique VLAN
     */
     UI16_T  vid;

     /* 0 -> not a  router port
      * 1 -> router port, include dynamic and static
      */
     UI8_T   router_port_bitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

     /* 0 -> not static router port
      * 1 -> static router port
      */
     UI8_T   static_router_port_bitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
} MLDSNP_OM_RouterPortList_T;

/*the port join entry struct. record the port running join entry attributes
  */
typedef struct MLDSNP_OM_PortInfo_S
{
    UI16_T      port_no;
    UI16_T      specific_query_count;                 /* indicate how many g-s or g-s-s query sent from this port*/

    MLDSNP_TYPE_JoinType_T join_type;                 /*none, unknown, dynamic, static*/
    MLDSNP_TYPE_ListType_T list_type;                 /*request list or exclude list*/

    UI32_T                 last_report_time;          /*record the time that report last reported, for stop g-s query used*/
    UI32_T                 rexmt_time_stamp;          /*record the time that retranmist packet*/
    UI32_T                 register_time;             /*record the time that this entry created*/
    MLDSNP_Timer_T         *ver1_host_present_timer_p;/*9.13*/
    struct L_list          host_sip_lst;

    union /*src_timer and filter timer only will exist one */
    {
        MLDSNP_Timer_T  *src_timer_p;                 /*this port's timer, the last time for ageout*/
        MLDSNP_Timer_T  *filter_timer_p;              /*for filter timer to chagne include or exclude mode*/
    };
} MLDSNP_OM_PortInfo_T;

/*running router port attributes
  */
typedef struct MLDSNP_OM_RouterPortInfo_S
{
    UI16_T                   port_no;

    UI32_T                   register_time;        /*record the time when the router port jion*/

    MLDSNP_TYPE_JoinType_T   attribute;            /*static dynamic*/

    MLDSNP_Timer_T           *router_timer_p;      /*this router's timer*/
} MLDSNP_OM_RouterPortInfo_T;

typedef struct MLDSNP_OM_VlanInfo_S
{
    UI16_T                         vid;

    /*MLD v2 has this vlaue, because the querier may run at each vlan, so the operation
       value may different in defferent vlan
      */
    UI16_T                         query_oper_interval;
    UI16_T                         robust_oper_value;
    BOOL_T                         s_flag;
    BOOL_T                         old_ver_querier_exist;
    UI8_T                            old_ver_querier_src_ip[MLDSNP_TYPE_IPV6_SRC_IP_LEN]; /*now only support record one*/
    UI8_T                          other_querier_src_ip[MLDSNP_TYPE_IPV6_SRC_IP_LEN]; /*now only support record one*/
    UI8_T                          last_reporter[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
    MLDSNP_TYPE_QuerierStatus_T    querier_runing_status;       /*indicate this vlan is running querier*/
    MLDSNP_TYPE_ImmediateStatus_T  immediate_leave_status;
    MLDSNP_TYPE_ImmediateByHostStatus_T  immediate_leave_byhost_status;
    UI32_T                         other_querier_uptime;
    UI32_T                         querier_uptime;    
    /*this querier's timer, this timer used to check other querier present or not*/
    MLDSNP_Timer_T                 *other_querier_present_timer_p;
    /* element is MLDSNP_OM_RouterPortInfo_T */
    L_SORT_LST_List_T              router_port_list;
} MLDSNP_OM_VlanInfo_T;

/*hisam entry struct
  */
typedef struct MLDSNP_OM_HisamEntry_S
{
    UI32_T                vid;
    UI8_T                 gip_a[MLDSNP_TYPE_IPV6_DST_IP_LEN];
    UI8_T                 sip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN];

    UI32_T                last_fwd_to_router_time;        /*the last time forward report to router*/
    L_SORT_LST_List_T     register_port_list;             /*element is MLDSNP_OM_PortInfo_T, link list the port join this entry*/

}MLDSNP_OM_HisamEntry_T;

/* record the UI configuration
 */
typedef struct MLDSNP_OM_StaticPortJoin_S
{
    UI16_T                 vid;
    UI16_T                 lport;
    UI8_T                  gip_a[MLDSNP_TYPE_IPV6_DST_IP_LEN];
    UI8_T                  sip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
    MLDSNP_TYPE_ListType_T list_type;
    BOOL_T                 is_used;
}MLDSNP_OM_StaticPortJoin_T;


/* MLD profile controlled group */
typedef struct MLDSNP_OM_ProfileGroupEntry_S
{
    /* the start multicast IP address of the controlled range */
    UI8_T  start_mip[SYS_ADPT_IPV6_ADDR_LEN];

    /* the start multicast IP address of the controlled range */
    UI8_T  end_mip[SYS_ADPT_IPV6_ADDR_LEN];

    /* The next controlled group range entry */
    struct MLDSNP_OM_ProfileGroupEntry_S  *next;

} MLDSNP_OM_ProfileGroupEntry_T;

/* MLD profile information */
typedef struct MLDSNP_OM_ProfileInfo_S
{
    /* The profile number */
    UI32_T  pid;

    /* The access mode of MLD profile   */
    /* MLDSNP_MGR_PROFILE_ACCESS_DENY   */
    /* MLDSNP_MGR_PROFILE_ACCESS_PERMIT */
    UI32_T  access_mode;

    /* The controlled group range entry */
    MLDSNP_OM_ProfileGroupEntry_T  *group_entry_p;

    /* The next profile entry */
    struct MLDSNP_OM_ProfileInfo_S  *nextPtr;
    UI32_T total_group_ranges;
} MLDSNP_OM_ProfileInfo_T;

/* MLD filter information */
typedef struct MLDSNP_OM_FilterInfo_S
{
    /* The MLD filter status               */
    /* MLDSNP_MGR_FILTER_STATUS_DISABLE    */
    /* MLDSNP_MGR_FILTER_STATUS_ENABLE     */
    UI32_T  filter_status;

    /* The profile entry */
    MLDSNP_OM_ProfileInfo_T    *profile_entry_p;
    UI32_T total_entry;
} MLDSNP_OM_FilterInfo_T;

/* The throttling information of the port */
typedef struct MLDSNP_OM_Throttle_S
{
    /* Throttle status */
    /* MLDSNP_MGR_THROTTLE_RUNNING_STATUS_FALSE */
    /* MLDSNP_MGR_THROTTLE_RUNNING_STATUS_TRUE  */
    UI32_T throttle_status;

    /* the throttling action                */
    /* MLDSNP_MGR_THROTTLE_ACTION_DENY     */
    /* MLDSNP_MGR_THROTTLE_ACTION_REPLACE  */
    UI32_T  action;

    /* The max-group number */
    UI32_T  max_group_number;

    /* The current joined group count */
    UI32_T current_group_count;

} MLDSNP_OM_Throttle_T;


typedef struct
{
    UI32_T port;
    MLDSNP_OM_Throttle_T throttling_info;
} MLDSNP_OM_IPCMsg_MldSnoopThrottleInfo_T;

typedef struct
{
    UI32_T mldsnp_Profile_id;
    UI32_T port;
} MLDSNP_OM_IPCMsg_MldSnoopProfilePort_T;

typedef struct
{
    UI32_T mldsnp_Profile_id;
    UI8_T  ip_begin[SYS_ADPT_IPV6_ADDR_LEN];
    UI8_T  ip_end[SYS_ADPT_IPV6_ADDR_LEN];
} MLDSNP_OM_IPCMsg_MldSnoopProfileRange_T;

typedef struct
{
    UI32_T mldsnp_Profile_id;
    UI32_T profile_mode;
} MLDSNP_OM_IPCMsg_MldSnoopProfileMode_T;

typedef struct
{
    UI32_T mldsnp_Profile_id;
} MLDSNP_OM_IPCMsg_MldSnoopProfile_T;

typedef struct
{
    UI32_T mldsnp_FilterStatus;
} MLDSNP_OM_IPCMsg_MldSnoopFilter_T;

typedef struct
{
    UI32_T value1;
} MLDSNP_OM_IPCMsg_GS1_T;

typedef struct
{
    UI32_T value1;
    UI32_T value2;
} MLDSNP_OM_IPCMsg_GS2_T;

typedef struct
{
    UI32_T value1;
    UI32_T value2;
    UI32_T value3;
} MLDSNP_OM_IPCMsg_GS3_T;

typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;   /*respond i32 return*/
    }type;

    union
    {

        BOOL_T bool_v;
        UI8_T  ui8_v;
        I8_T   i8_v;
        UI32_T ui32_v;
        UI16_T ui16_v;
        I32_T  i32_v;
        I16_T  i16_v;
        UI8_T  ip4_v[4];

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
        }u32a1_u32a2;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
        }u32a1_u32a2_u32_a3;

        struct
        {
            UI16_T  vid;
            UI8_T   gip_ar[MLDSNP_TYPE_IPV6_DST_IP_LEN];
            UI8_T   sip_ar[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
            MLDSNP_OM_GroupInfo_T  scgroup_info;
        }group_portlist;

        struct
        {
            UI16_T  vid;
            MLDSNP_TYPE_HisamKeyIndex_T  key_idx;
            UI8_T   gip_ar[MLDSNP_TYPE_IPV6_DST_IP_LEN];
            UI8_T   sip_ar[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
            MLDSNP_OM_HisamEntry_T  entry_info;
        }hisam_entry_info;

        struct
        {
            UI16_T  vid;
            UI16_T  port_no;
            UI8_T   gip_ar[MLDSNP_TYPE_IPV6_DST_IP_LEN];
            UI8_T   sip_ar[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
            MLDSNP_OM_PortInfo_T  port_info;
        }port_info;

        struct
        {
            UI16_T  vid;
            UI16_T  r_port;
            MLDSNP_OM_RouterPortInfo_T  router_port_info;
        }router_port_info;

        struct
        {
            UI16_T  id;
            UI16_T  vid;
            UI16_T port;
            UI8_T   gip_ar[MLDSNP_TYPE_IPV6_DST_IP_LEN];
            UI8_T   sip_ar[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
            MLDSNP_TYPE_RecordType_T rec_type;
        }running_join_static_group;

        struct
        {
            UI16_T vid;
            UI8_T  router_port_bitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
        }running_router_portlist;

        struct
        {
            UI16_T vid;
            MLDSNP_OM_RouterPortList_T  router_port_list;
        }vlan_router_portlist;

        struct
        {
            UI16_T vid;
            MLDSNP_TYPE_ImmediateStatus_T status;
            MLDSNP_TYPE_ImmediateByHostStatus_T byhost_status;
        }immediate_leave;

        struct
        {
            UI16_T vid;
            MLDSNP_TYPE_ImmediateByHostStatus_T status;
        }immediate_leave_byhost;

        struct
        {
            UI16_T vid;
            MLDSNP_TYPE_QuerierStatus_T status;
        }running_queier;
        struct
        {
          UI32_T vlan_id;
          MLDSNP_TYPE_UnknownBehavior_T flood_behavior;
        }flood;
        struct
        {
          UI32_T vlan_id;
          MLDSNP_OM_VlanRunningCfg_T vlan_cfg;
        }vlan_cfg;

        MLDSNP_TYPE_MLDSNP_STATUS_T mldsnp_status;
        MLDSNP_TYPE_QuerierStatus_T querier_status;

        MLDSNP_OM_RunningCfg_T mldsnp_running_global_conf;
        MLDSNP_OM_Cfg_T  mldsnp_global_conf;
        MLDSNP_OM_PortSourceListInfo_T port_source_list;
        MLDSNP_OM_PortSourceInfo_T  port_grp_src;
        MLDSNP_OM_PortHostInfo_T port_host;
        MLDSNP_TYPE_ProxyReporting_T proxy_reporting;
        MLDSNP_OM_IPCMsg_GS3_T u32_3;
    }data;
}   MLDSNP_OM_IPCMsg_T;

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/


/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/*---------------------------------------------------------------------------------
 * FUNCTION : MLDSNP_OM_LinkListNodeFree
 *---------------------------------------------------------------------------------
 * PURPOSE  : Free the memalloc when insert a node to linklist
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void MLDSNP_OM_LinkListNodeFree(void *p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRunningGlobalConf
*------------------------------------------------------------------------------
* Purpose: This function get the running configuration
* INPUT  : None
* OUTPUT : *mldsnp_global_conf_p - the global configuration
* RETUEN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS    - not same as default
*          SYS_TYPE_GET_RUNNING_CFG_FAIL	   - get failure
*          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - same as default
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_GetRunningGlobalConf(
                                MLDSNP_OM_RunningCfg_T *mldsnp_global_conf_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRunningRouterPortList
*------------------------------------------------------------------------------
* Purpose: This function get next the run static router port bit list
* INPUT  : vid - the vlan id to get next vlan id
*
* OUTPUT :  *router_port_bitmap_p  - the router port bit map pointer
*
* RETUEN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default
*           SYS_TYPE_GET_RUNNING_CFG_FAIL	   - get failure
*           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default
* NOTES  :
*------------------------------------------------------------------------------*/

UI32_T MLDSNP_OM_GetRunningRouterPortList(
                                          UI16_T vid,
                                          UI8_T *router_port_list_ap);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRunningVlanImmediateStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate running status
* INPUT  : vid  - the vlan id
*
* OUTPUT : *status_p   - the immediate status
* RETUEN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default
*           SYS_TYPE_GET_RUNNING_CFG_FAIL	   - get failure
*           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_GetRunningVlanImmediateStatus(
                                               UI16_T                        vid,
                                               MLDSNP_TYPE_ImmediateStatus_T *imme_status_p,
                                               MLDSNP_TYPE_ImmediateByHostStatus_T *imme_byhost_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the group's port list
* INPUT  : vid     - the vlan id
*          *gip_p  - the group ip address
*          *sip_p  - the source ip address
*
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : *group_info_p  - the group informaiton
* NOTES  : This function is only provide to UI to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetGroupPortlist(
                                  UI16_T                vid,
                                  UI8_T                 *gip_p,
                                  UI8_T                 *sip_p,
                                  MLDSNP_OM_GroupInfo_T *group_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the next group's port list
* INPUT  : *vid_p    - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT : *vid_p   - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
*          *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetNextGroupPortlist(
                                      UI16_T                *vid_p,
                                      UI8_T                 *gip_ap,
                                      UI8_T                 *sip_ap,
                                      MLDSNP_OM_GroupInfo_T *group_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetGroupSourcePortlist
*------------------------------------------------------------------------------
* Purpose: This function get the group and source  port list
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT : *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : 1. This function is only rovide to UI to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetGroupSourcePortlist(
                                  UI16_T                vid,
                                  UI8_T                 *gip_ap,
                                  UI8_T                 *sip_ap,
                                  MLDSNP_OM_GroupInfo_T *group_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextGroupSourcePortlist
*------------------------------------------------------------------------------
* Purpose: This function get the next group and source  port list
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT : *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : 1. This function is only rovide to UI to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetNextGroupSourcePortlist(
                                  UI16_T                vid,
                                  UI8_T                 *gip_ap,
                                  UI8_T                 *sip_ap,
                                  MLDSNP_OM_GroupInfo_T *group_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextPortGroupSourceList
*------------------------------------------------------------------------------
* Purpose: This function get the port joined all group
* INPUT  : port_src_lst_p->vid    - the next vlan id
*          port_src_lst_p->port  - the port to get
* OUTPUT :  port_src_lst_p - the next vid, gip and source list
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetNextPortGroupSourceList(
                                        MLDSNP_OM_PortSourceListInfo_T *port_src_lst_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextPortGroupSourceList
*------------------------------------------------------------------------------
* Purpose: This function get the port joined all group
* INPUT  : port_src_lst_p->vid    - the next vlan id
*          port_src_lst_p->port  - the port to get
* OUTPUT :  port_src_lst_p - the next vid, gip and source list
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*              get the port joined group and source list,
*              (port, vid, group) is the key
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetPortGroupSourceList(
                                        MLDSNP_OM_PortSourceListInfo_T *port_src_lst_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextVlanRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the vlan's router list
* INPUT  : *vid_p           - the vlan id
* OUTPUT : router_port_list - the router port info
*         *vid_p            - the next vid
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This function is only provide to UI to access
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextVlanRouterPortlist(
                                           UI16_T                     *vid_p,
                                           MLDSNP_OM_RouterPortList_T *router_port_list);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetVlanRouterPortCount
*------------------------------------------------------------------------------
* Purpose: This function get the vlan's router port count
* INPUT  : vid               - the vlan id
* OUTPUT : None
* RETURN : the count of router port in this vlan
* NOTES  :
*------------------------------------------------------------------------------*/
UI16_T MLDSNP_OM_GetVlanRouterPortCount(
                                      UI16_T vid);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetGlobalConf
*------------------------------------------------------------------------------
* Purpose: This function get the global configuration
* INPUT  : None
*
* OUTPUT : *mldsnp_global_conf_p - the global configuration
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetGlobalConf(
                            MLDSNP_OM_Cfg_T *mldsnp_global_conf_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetMldStatus
*------------------------------------------------------------------------------
* Purpose: This function get the Mld status
* INPUT  :   None
* OUTPUT : *mldsnp_status_p - the mldsnp status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetMldStatus(
                            MLDSNP_TYPE_MLDSNP_STATUS_T *mldsnp_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetMldStatus
*------------------------------------------------------------------------------
* Purpose: This function set the Mld status
* INPUT  : mldsnp_status - the mldsnp status
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetMldStatus(
                             MLDSNP_TYPE_MLDSNP_STATUS_T mldsnp_status);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetMldSnpVer
*------------------------------------------------------------------------------
* Purpose: This function get the mldsno version
* INPUT  : None
* OUTPUT : *ver_p - the version
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetMldSnpVer(
                             UI16_T*ver_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetMldSnpVer
*------------------------------------------------------------------------------
* Purpose: This function set the mldsno version
* INPUT  : ver
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetMldSnpVer(
                            MLDSNP_TYPE_Version_T ver);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_StoreQuerierTimer
*------------------------------------------------------------------------------
* Purpose: This function store the querier timer pointer
* INPUT  : *timer_p  - the querier timer pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_StoreQuerierTimer(
                                MLDSNP_Timer_T *timer_p);
 /*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetQuerierTimer
*------------------------------------------------------------------------------
* Purpose: This function get the querier timer pointer
* INPUT  : **timer_p - the querier timer pointer pointer
* OUTPUT : **timer_p - the qurerier timer pointer
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_GetQuerierTimer(
                            MLDSNP_Timer_T **timer_p);
 /*------------------------------------------------------------------------------
* Function : MLDSNP_OM_StoreMrdSolicitationTimer
*------------------------------------------------------------------------------
* Purpose: This function store the MRD solicitation timer pointer
* INPUT  : *timer_p  - the querier timer pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_StoreMrdSolicitationTimer(
                            MLDSNP_Timer_T *timer_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetMrdSolicitationTimer
*------------------------------------------------------------------------------
* Purpose: This function get the mrd solicitation timer pointer
* INPUT  : **timer_p - the querier timer pointer pointer
* OUTPUT : **timer_p - the qurerier timer pointer
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_GetMrdSolicitationTimer(
                            MLDSNP_Timer_T **timer_p);

 /*------------------------------------------------------------------------------
* Function : MLDSNP_OM_StoreVlanOtherQuerierPresentTimer
*------------------------------------------------------------------------------
* Purpose: This function store the vlan's querier timer pointer
* INPUT  : vid       - the vlan id
*          *timer_p  - the querier timer pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : The vlan's querier timer used to indicate other querier present timer
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_StoreVlanOtherQuerierPresentTimer(
                                                   UI16_T    vid,
                                                   MLDSNP_Timer_T *timer_p);
 /*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetVlanOtherQuerierPresentTimer
*------------------------------------------------------------------------------
* Purpose: This function get the querier timer pointer
* INPUT  : **timer_p - the querier timer pointer pointer
* OUTPUT : **timer_p - the qurerier timer pointer
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetVlanOtherQuerierPresentTimer(
                                                 UI16_T    vid,
                                                 MLDSNP_Timer_T **timer_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetQuerierStauts
*------------------------------------------------------------------------------
* Purpose: This function get the querier status
* INPUT  : None
* OUTPUT : *querier_status_p - the querier status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetQuerierStauts(
                                MLDSNP_TYPE_QuerierStatus_T *querier_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetQuerierStatus
*------------------------------------------------------------------------------
* Purpose: This function set the querier status
* INPUT  : querier_status - the querier status
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetQuerierStatus(
                                MLDSNP_TYPE_QuerierStatus_T querier_status);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetMroutStatus
*------------------------------------------------------------------------------
* Purpose: This function set the mroute status
* INPUT  : is_enabled - mroute is enabled or not
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetMroutStatus(BOOL_T is_enabled);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_IsMrouteEnabled
*------------------------------------------------------------------------------
* Purpose: This function check the mroute enabled
* INPUT  : is_enabled - mroute is enabled or not
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_IsMrouteEnabled();

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRobustnessValue
*------------------------------------------------------------------------------
* Purpose: This function get the robust ess value
* INPUT  : None
* OUTPUT : *value_p - the robustness value
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.1, 7.9, 9.1, 9.9
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetRobustnessValue(
                                UI16_T* value_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRobustnessOperValue
*------------------------------------------------------------------------------
* Purpose: This function get the robustness operate value
* INPUT  : None
* OUTPUT : *value_p - the robustness value
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.1, 7.9, 9.1, 9.9
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetRobustnessOperValue(
                                        UI16_T vid,
                                        UI16_T *value_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetRobustnessValue
*------------------------------------------------------------------------------
* Purpose: This function set the robust ess value
* INPUT  : value - the robustness value
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.1, 7.9, 9.1, 9.9
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetRobustnessValue(
                                UI16_T value);


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetQueryInterval
*------------------------------------------------------------------------------
* Purpose: This funcgtion get the query interval
* INPUT  : None
* OUTPUT : *interval_p - the interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :   7.2, 9.2
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetQueryInterval(
                                UI16_T* interval_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetQueryInterval
*------------------------------------------------------------------------------
* Purpose: This funcgtion get the query interval
* INPUT  : None
* OUTPUT : *interval_p - the interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :   7.2, 9.2
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetQueryOperInterval(
                                    UI16_T vid,
                                    UI16_T *interval_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetQueryInterval
*------------------------------------------------------------------------------
* Purpose: This funcgtion set the query interval
* INPUT  : interval - the interval in seconds
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.2, 9.2
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetQueryInterval(
                                UI16_T interval);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetQueryResponseInterval
*------------------------------------------------------------------------------
* Purpose: This function get the query response interval
* INPUT  : None
* OUTPUT : *interval_p  - the query response interval
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :   7.3, 9.3
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetQueryResponseInterval(
                                        UI16_T* interval_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetQueryRresponseInterval
*------------------------------------------------------------------------------
* Purpose: This function get the query response interval
* INPUT  : interval - the query repsonse interval in seconds
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :   7.3, 9.3
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetQueryRresponseInterval(
                                        UI16_T interval);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetListenerInterval
*------------------------------------------------------------------------------
* Purpose: This function get the listner interval
* INPUT  : None
* OUTPUT : *interval_p - the listerner interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :7.4 in RFC2710,  9.4 in RFC 3810
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetListenerInterval(
                                    UI16_T* interval_p);

 /*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetStartupQueryInterval
*------------------------------------------------------------------------------
* Purpose: This function get the querier startup query interval
* INPUT  : None
* OUTPUT : *interval_p - the listerner interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :7.6 in RFC2710,  9.6 in RFC 3810
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetStartupQueryInterval(
                                        UI16_T* interval_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetStartupQueryCount
*------------------------------------------------------------------------------
* Purpose: This function get the querier startup query count
* INPUT  : None
* OUTPUT : *query_count_p - the listerner interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :7.7 in RFC2710,  9.7 in RFC 3810
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetStartupQueryCount(
                                    UI16_T* query_count_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetStartupQuerySentCount
*------------------------------------------------------------------------------
* Purpose: This function set the queris querier startup has been sent
* INPUT  : None
* OUTPUT : *query_count_p - the listerner interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :7.7 in RFC2710,  9.7 in RFC 3810
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetStartupQuerySentCount(
                                        UI16_T query_count_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetStartupQuerySentCount
*------------------------------------------------------------------------------
* Purpose: This function get the query querier startup has been sent
* INPUT  : None
* OUTPUT : *query_count_p - the listerner interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :7.7 in RFC2710,  9.7 in RFC 3810
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetStartupQuerySentCount(
                                        UI16_T* query_count_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetOtherQueryPresentInterval
*------------------------------------------------------------------------------
* Purpose: This function get the other querier present interval
* INPUT  : None
* OUTPUT :None
* RETURN : other querier present interval
* NOTES  :  7.5, 9.5 robust * query_interval + query_rsponse_interval
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_GetOtherQueryPresentInterval();

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetOldVerQuerierPresentTimeOut
*------------------------------------------------------------------------------
* Purpose: This function get the old version querier present time out value
* INPUT  : vid	- the vlan id

* OUTPUT : *time_out_p   - the time out in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :9.12 in RFC3180
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetOldVerQuerierPresentTimeOut(
                                            UI16_T* time_out_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetMALI
*------------------------------------------------------------------------------
* Purpose: This function get MALI value
* INPUT  :  vid - the vlan id
* OUTPUT : None
* RETURN : MALI vaule
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_GetMALI(
                        UI16_T vid);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetLastListenerQueryInterval
*------------------------------------------------------------------------------
* Purpose: This function Get the last listener query interval
* INPUT  : None
* OUTPUT : *interval_p  - the interval in second
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.8, 9.8
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetLastListenerQueryInterval(
                                            UI16_T* interval_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetLastListnerQueryInterval
*------------------------------------------------------------------------------
* Purpose: This function set the last listener query interval
* INPUT  : interval  - the interval in second
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.8, 9.8
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetLastListnerQueryInterval(
                                            UI16_T interval);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetUnsolicitTimer
*------------------------------------------------------------------------------
* Purpose: This function store the querier timer pointer
* INPUT  : *timer_p  - the querier timer pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_SetUnsolicitTimer(
    MLDSNP_Timer_T *timer_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetUnsolicitTimer
*------------------------------------------------------------------------------
* Purpose: This function get the querier timer pointer
* INPUT  : **timer_p - the querier timer pointer pointer
* OUTPUT : **timer_p - the qurerier timer pointer
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_GetUnsolicitTimer(
    MLDSNP_Timer_T **timer_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetProxyReporting
*------------------------------------------------------------------------------
* Purpose: This function Get the proxy switching
* INPUT  : None
* OUTPUT : *proxy_staus_p  - the proxy reporting status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.9, 9.11
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetProxyReporting(MLDSNP_TYPE_ProxyReporting_T *proxy_staus_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetProxyReporting
*------------------------------------------------------------------------------
* Purpose: This function Set the proxy reporting status
* INPUT  : proxy_staus_p  - the proxy reporting status
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.9, 9.11
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetProxyReporting(MLDSNP_TYPE_ProxyReporting_T proxy_staus);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetUnsolicitedReportInterval
*------------------------------------------------------------------------------
* Purpose: This function Get the unsolicitedReportInterval
* INPUT  : None
* OUTPUT : *interval_p  - the inteval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.9, 9.11
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetUnsolicitedReportInterval(
    UI32_T* interval_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetUnsolicitedReportInterval
*------------------------------------------------------------------------------
* Purpose: This function set the unsolicitedReportInterval
* INPUT  : interval  - the inteval in seconds
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.9, 9.11
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetUnsolicitedReportInterval(
                                            UI16_T interval);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetLastReportHostAddress
*------------------------------------------------------------------------------
* Purpose: This function set last reporter host IP address
* INPUT  : vid   - which vlan
*          src_ap- source ip address
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetLastReportHostAddress(UI16_T vid, UI8_T *src_ap);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetLastReportHostAddress
*------------------------------------------------------------------------------
* Purpose: This function get last reporter host IP address
* INPUT  : vid   - which vlan
* OUTPUT : src_ap- source ip address
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetLastReportHostAddress(UI16_T vid, UI8_T *src_ap);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRouterExpireTime
*------------------------------------------------------------------------------
* Purpose: This function get the router expire time
* INPUT  :
* OUTPUT : exp_time_p  - the expire time
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetRouterExpireTime(
                                UI16_T* exp_time_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetRouterExpireTime
*------------------------------------------------------------------------------
* Purpose: This function set the router expire time
* INPUT  : exp_time  - the expire time
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetRouterExpireTime(
                                    UI16_T exp_time);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate leave status
* INPUT  : vid                      - the vlan id
*
* OUTPUT : immediate_leave_status_p - the immediate leave status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetImmediateLeaveStatus(
                                         UI16_T                        vid,
                                         MLDSNP_TYPE_ImmediateStatus_T *immediate_leave_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetImmediateLeaveByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate leave by-host-ip status
* INPUT  : vid                      - the vlan id
*
* OUTPUT : immediate_leave_byhost_status_p - the immediate leave status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetImmediateLeaveByHostStatus(
                                        UI16_T                        vid,
                                        MLDSNP_TYPE_ImmediateByHostStatus_T *immediate_leave_byhost_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate leave status
* INPUT  : *vid_p                   - the vlan id
* OUTPUT : immediate_leave_status_p - the immediate leave status
*          *vid_p                   - the next vid
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextImmediateLeaveStatus(
                                             UI16_T                        *vid_p,
                                             MLDSNP_TYPE_ImmediateStatus_T *immediate_leave_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextImmediateLeaveByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate leave by-host-ip status
* INPUT  : *vid_p                   - the vlan id
* OUTPUT : immediate_leave_byhost_status_p - the immediate leave by-host-ip status
*          *vid_p                   - the next vid
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextImmediateLeaveByHostStatus(
                                             UI16_T                        *vid_p,
                                             MLDSNP_TYPE_ImmediateByHostStatus_T *immediate_leave_byhost_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function Set the immediate leave status
* INPUT  : vid                    - the vlan id
*          immediate_leave_status - the returned router port info
*
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetImmediateLeaveStatus(
                                         UI16_T                        vid,
                                         MLDSNP_TYPE_ImmediateStatus_T immediate_leave_status);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetImmediateLeaveByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function Set the immediate leave by-host-ip status
* INPUT  : vid                           - the vlan id
*          immediate_leave_byhost_status - the immediate leave by-host-ip status
*
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetImmediateLeaveByHostStatus(
                                        UI16_T                        vid,
                                        MLDSNP_TYPE_ImmediateByHostStatus_T immediate_leave_byhost_status);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function Set the unknown multicast data flood behavior
* INPUT  :  flood_behavior - the returned router port info
*          vlan_id         - which vlan
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetUnknownFloodBehavior(
    UI32_T vlan_id,
                                        MLDSNP_TYPE_UnknownBehavior_T flood_behavior);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function get the unknown multicast data flood behavior
* INPUT  : flood_behavior_p - the returned router port info
*          vlan_id         - which vlan
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetUnknownFloodBehavior(
    UI32_T vlan_id,
                                       MLDSNP_TYPE_UnknownBehavior_T *flood_behavior_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetS_QRV_QQIC
*------------------------------------------------------------------------------
* Purpose: This function set the querier S flag value
* INPUT  :  flood_behavior_p - the returned router port info
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_SetS_QRV_QQIC(
                            UI16_T vid,
                            BOOL_T s_on,
                            UI16_T robust_value,
                            UI16_T query_interval);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetV1HostPresentPortbitmap
*------------------------------------------------------------------------------
* Purpose: This function get the v1 host present portbitmap
* INPUT  :  None
* OUTPUT : portbitmap - the portbitmap array to put the result
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_GetV1HostPresentPortbitmap(
                                        UI8_T *portbitmap_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_AddV1HostPresentPort
*------------------------------------------------------------------------------
* Purpose: This function add the v1 host present port
* INPUT  :  lport - the port to add to portbitmap
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_AddV1HostPresentPort(
                                    UI16_T lport);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteV1HostPresentPort
*------------------------------------------------------------------------------
* Purpose: This function delete the v1 host present port
* INPUT  :  lport - the port to add to portbitmap
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_DeleteV1HostPresentPort(
                                        UI16_T lport);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_AddStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function add the static join group entry
* INPUT  :  vid - vlan id
*               gip_ap - group array pointer
*               sip_ap - source array pointer
*               lport   - registered port
*               rec_type - exclude or include
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_AddStaticPortJoinGroup(
                                    UI16_T vid,
                                    UI8_T *gip_ap,
                                    UI8_T *sip_ap,
                                    UI16_T lport,
                                    MLDSNP_TYPE_RecordType_T rec_type);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_IsExistStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function check the group is already joined before
* INPUT  : vid - vlan id
*          gip_ap - group array pointer
*          sip_ap - source array pointer
*          lport   - registered port
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_IsExistStaticPortJoinGroup(
                                    UI16_T vid,
                                    UI8_T *gip_ap,
                                    UI8_T *sip_ap,
                                    UI16_T lport);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function get next static join group entry
* INPUT  :  next_id  - the next used index in array
*               out_vid - vlan id
*               out_gip_ap - group array pointer
*               out_sip_ap - source array pointer
*               out_lport   - registered port
*               out_rec_type - exclude or include
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextStaticPortJoinGroup(
                                        UI16_T *next_id_p,
                                        UI16_T *out_vid_p,
                                        UI8_T *out_gip_ap,
                                        UI8_T *out_sip_ap,
                                        UI16_T *out_lport,
                                        MLDSNP_TYPE_RecordType_T *out_rec_type_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextRunningStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function get next static join group entry
* INPUT  :  next_id  - the next used index in array
*               out_vid - vlan id
*               out_gip_ap - group array pointer
*               out_sip_ap - source array pointer
*               out_lport   - registered port
*               out_rec_type - exclude or include
* OUTPUT : None
* RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- success
*          SYS_TYPE_GET_RUNNING_CFG_FAIL -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_GetNextRunningStaticPortJoinGroup(
                                        UI16_T *next_id_p,
                                        UI16_T *out_vid_p,
                                        UI8_T *out_gip_ap,
                                        UI8_T *out_sip_ap,
                                        UI16_T *out_lport,
                                        MLDSNP_TYPE_RecordType_T *out_rec_type_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the next static join group port bit list
* INPUT  : vid_p    - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT :  vid_p   - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
*          *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : 
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetNextStaticGroupPortlist(
    UI16_T                *vid_p,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    UI8_T                 *static_portlist);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetStaticGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the static join group port bit list
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT : *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : 
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetStaticGroupPortlist(
    UI16_T                vid,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    UI8_T                 *static_portlist);


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function add the static join group entry
* INPUT  :  vid - vlan id
*               gip_ap - group array pointer
*               sip_ap - source array pointer
*               lport   - registered port
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteStaticPortJoinGroup(
                                    UI16_T vid,
                                    UI8_T *gip_ap,
                                    UI8_T *sip_ap,
                                    UI16_T lport);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_ReplaceStaticJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function replace the static join group between port and port-channel
* INPUT  : from_ifindex - the port to be replaced
*          to_ifindex - replace by which port
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_ReplaceStaticJoinGroup(
    UI16_T id,
    UI32_T  from_ifindex,
    UI32_T  to_ifindex );

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteAllStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function delete all the static group joined on that port
* INPUT  : lport   - registered port
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteAllStaticPortJoinGroup(
    UI16_T lport);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteAllStaticPortJoinGroupInVlan
*------------------------------------------------------------------------------
* Purpose: This function delete all the static group joined on that port in that vlan
* INPUT  : vid - vlan id
*          lport   - registered port
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteAllStaticPortJoinGroupInVlan(
    UI16_T vid,
    UI16_T lport);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteAllStaticJoinGroupInVlan
*------------------------------------------------------------------------------
* Purpose: This function delete all the static group joined in that vlan
* INPUT  : vid - vlan id
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteAllStaticJoinGroupInVlan(
    UI16_T vid);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetQuerierRunningStatus
*------------------------------------------------------------------------------
* Purpose: This function set the querier running status of the input vlan
* INPUT  : vid    - the vlan id
*          status - the querier status
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetQuerierRunningStatus(
                                         UI16_T vid,
                                         MLDSNP_TYPE_QuerierStatus_T status);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetQuerierRunningStatus
*------------------------------------------------------------------------------
* Purpose: This function get the querier running status of the input vlan
* INPUT  : vid  - the vlan id
* OUTPUT : *status_p  - the querier running status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetQuerierRunningStatus(
                                         UI16_T vid,
                                         MLDSNP_TYPE_QuerierStatus_T *status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_InitialAllVlanQuerierRunningStatus
*------------------------------------------------------------------------------
* Purpose: This function set the querier running status of the input vlan
* INPUT  : runining_status - the initial querier running status
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_InitialAllVlanQuerierRunningStatus(
                                            MLDSNP_TYPE_QuerierStatus_T runining_status);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteAllPortFromPortList
*------------------------------------------------------------------------------
* Purpose:This function add the port into group entry's port list
* INPUT  : *hisam_entry_p  - the group entry
*
* OUTPUT : None
* RETURN : TRUE - success
*        FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteAllPortFromPortList(
                                        MLDSNP_OM_HisamEntry_T *hisam_entry_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_Init
*------------------------------------------------------------------------------
* Purpose: This function initial the OM
* INPUT  : None
*
*
* OUTPUT : None
* RETURN : TRUE - success
*       FALSE- fail
* NOTES  : clear om can't use this function
*------------------------------------------------------------------------------*/
void MLDSNP_OM_Init();
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_EnterTransitionMode
*------------------------------------------------------------------------------
* Purpose: This function clear all the om record
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  : clear om record
*------------------------------------------------------------------------------*/
void MLDSNP_OM_EnterTransitionMode();
 /*------------------------------------------------------------------------------
* Function : MLDSNP_OM_EnterMasterMode
*------------------------------------------------------------------------------
* Purpose: This function set the default value
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  : None
*------------------------------------------------------------------------------*/
void MLDSNP_OM_EnterMasterMode();

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_InitVlanInfo
*------------------------------------------------------------------------------
* Purpose: This function initial the vlan info
* INPUT  :   vid                     - the vlan id
*                *vid_info_p          - the vlan info pointer
* OUTPUT : vid_info_p  - the initialize hisam entry
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_InitVlanInfo(
                            UI16_T vid,
                            MLDSNP_OM_VlanInfo_T *vlan_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_InitialportInfo
*------------------------------------------------------------------------------
* Purpose: This function initial the port information
* INPUT  : lport        - the logical port
*          port_info_p  - the port info pointer
*          jon_type     - how the port join the group
*          register_tiem- the time initial this port
*          list_type    - put this port into which list type
* OUTPUT : *port_info_p - the initialize port info
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_InitialportInfo(
                               UI16_T                 lport,
                               MLDSNP_TYPE_JoinType_T join_type,
                               MLDSNP_TYPE_ListType_T list_type,
                               UI32_T register_time,
                               MLDSNP_OM_PortInfo_T   *port_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_InitialHisamEntry
*------------------------------------------------------------------------------
* Purpose: This function initial the hiam entry
* INPUT  : vid   - the vlan id
*         *gip_ap - the group ip
*         *sip_ap - the source ip
* OUTPUT : *hisam_entry_p  - the initialize hisam entry
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_InitialHisamEntry(
                                 UI16_T                 vid,
                                 UI8_T                  *gip_ap,
                                 UI8_T                  *sip_ap,
                                 MLDSNP_OM_HisamEntry_T *hisam_entry_p);
 /*------------------------------------------------------------------------------
 * Function : MLDSNP_OM_SetHisamEntryInfo
 *------------------------------------------------------------------------------
 * Purpose: This function set the info in hisam entry
 * INPUT  :  *entry_info   - the infomation store in hisam
 * OUTPUT :
 * RETURN : TRUE - success
 *			FALSE- fail
 * NOTES  :  MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC
 *------------------------------------------------------------------------------*/
 BOOL_T MLDSNP_OM_SetHisamEntryInfo(
                                 MLDSNP_OM_HisamEntry_T *entry_info);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteHisamEntryInfo
*------------------------------------------------------------------------------
* Purpose: This function set the info in hisam entry
* INPUT  :  *entry_info   - the infomation store in hisam
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : please make sure all register port's timer has been clear/freed
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteHisamEntryInfo(
                                MLDSNP_OM_HisamEntry_T *entry_info);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_AddPortInfo
*------------------------------------------------------------------------------
* Purpose: this function add the port info
*          if the group entry doen't exist, this entry will be created first.
* INPUT  : vid           - the vlan id
*          *gip_ap       - the group ip address
*          *sip_ap       - the source ip address
*          port_info_p   - the port info
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : if the hisam entry is not existed, the function will create the hisam entry
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_AddPortInfo(
                             UI16_T               vid,
                             UI8_T                *gip_ap,
                             UI8_T                *sip_ap,
                             MLDSNP_OM_PortInfo_T *port_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeletePortInfo
*------------------------------------------------------------------------------
* Purpose: This function delete the port from hisam
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
*          lport    - the logical port
* OUTPUT : None
* RETURN : TRUE    - success
*          FALSE   - fail
* NOTES  : if there is no port in the hisam entry, this hisam entry will be deleted
*         this function won't take care the pointer in portinfo, use this function shall make sure
*         all timer in port info already been stopped.
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_DeletePortInfo(
                                UI16_T vid,
                                UI8_T  *gip_ap,
                                UI8_T  *sip_ap,
                                UI16_T lport);
 /*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SortCompPortInfoPortNo
*------------------------------------------------------------------------------
* Purpose:
 * PURPOSE  : a function used in the remote management address sort list
 * INPUT    : *list_node_p        - the node in the sort list
 *            *comp_node_p        - the node to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
*------------------------------------------------------------------------------*/
int MLDSNP_OM_SortCompPortInfoPortNo(
                                     void* list_node_p,
                                     void* comp_node_p);

/*------------------------------------------------------------------------------
 * Function : MLDSNP_OM_SortCompRouterPortInfoPortNo
 *------------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote management address sort list
 * INPUT    : *list_node_p        - the node in the sort list
 *            *comp_node_p        - the node to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
*------------------------------------------------------------------------------*/
int MLDSNP_OM_SortCompRouterPortInfoPortNo(
                                           void* list_node_p,
                                           void* comp_node_p);
/*------------------------------------------------------------------------------
 * Function : MLDSNP_OM_SortCompVlanId
 *------------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote management address sort list
 * INPUT    : *list_node_p        - the node in the sort list
 *            *comp_node_p        - the node to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
*------------------------------------------------------------------------------*/
int MLDSNP_OM_SortCompVlanId(
                             void* list_node_p,
                             void* comp_node_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetHisamEntryInfo
*------------------------------------------------------------------------------
* Purpose: This function get the info in hisam entry
* INPUT  : vid       - the vlan id
*          gip_ap    - the group ip
*          sip_ap    - the source ip
*          key_idx   - use which key to get
* OUTPUT : *entry_info_p   - the infomation store in hisam
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetHisamEntryInfo(
                                   UI16_T                      vid,
                                   UI8_T                       *gip_ap,
                                   UI8_T                       *sip_ap,
                                   MLDSNP_TYPE_HisamKeyIndex_T key_idx,
                                   MLDSNP_OM_HisamEntry_T      *entry_info_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextHisamEntryInfo
*------------------------------------------------------------------------------
* Purpose: This function get the info in hisam entry
* INPUT  : *vid_p      - the vlan id
*          *gip_ap     - the group ip
*          *sip_ap     - the source ip
*          key_idx     - use which key to get
* OUTPUT : *vid_p      - the next vlan id
*          *gip_ap     - the next group ip
*          *sip_ap     - the next source ip
*          *entry_info_p- the infomation store in hisam
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : key idx = MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextHisamEntryInfo(
                                       UI16_T                      *vid_p,
                                       UI8_T                       *gip_ap,
                                       UI8_T                       *sip_ap,
                                       MLDSNP_TYPE_HisamKeyIndex_T key_idx,
                                       MLDSNP_OM_HisamEntry_T      *entry_info);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_RemoveAllGroupsInVlan
*------------------------------------------------------------------------------
* Purpose: This function will remove the specified vlan's all group
* INPUT  : vid  - the vlan id
*
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN :
* NOTES  :this function won't take care the link list in the entry
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_RemoveAllGroupsInVlan(
                                      UI16_T vid);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_RemoveAllHisamEntry
*------------------------------------------------------------------------------
* Purpose: This function will remove all hisam entry
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
*
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :this function won't take care the link list in the entry
*         it shall free the point in all entries first.
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_RemoveAllHisamEntry();

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_UpdatePortInfo
*------------------------------------------------------------------------------
* Purpose: This function is used to update the port info
* INPUT  : vid                   - the vlan id
*          *gip_ap               - the group ip
*          *sip_ap               - the source ip
*          *update_port_info_p   - the update port info
*
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  if the PortInfo not exit, it will return fail
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_UpdatePortInfo(
                                UI16_T               vid,
                                UI8_T                *gip_ap,
                                UI8_T                *sip_ap,
                                MLDSNP_OM_PortInfo_T *update_port_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_UpdatePortInfoFromHisam
*------------------------------------------------------------------------------
* Purpose: This function is used to update the port info
* INPUT  : *entry_info          - the hisam entry info
*          *update_port_info_p   - the update port info
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  if the PortInfo not exit, it will return fail
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_UpdatePortInfoFromHisam(
                                         MLDSNP_OM_HisamEntry_T *entry_info_p,
                                         MLDSNP_OM_PortInfo_T   *update_port_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetPortInfo
*------------------------------------------------------------------------------
* Purpose: This function get the port info
* INPUT  : vid                   - the vlan id
*          *gip_ap               - the group ip
*          *sip_ap               - the source ip
*          lport                 - the logical port
* OUTPUT : retrun_port_info_p    - the port info
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetPortInfo(
                             UI16_T               vid,
                             UI8_T                *gip_ap,
                             UI8_T                *sip_ap,
                             UI16_T               lport,
                             MLDSNP_OM_PortInfo_T *retrun_port_info_p);


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_AddPortHostIp
*------------------------------------------------------------------------------
* Purpose: This function  add the host ip to port
* INPUT  : vid                - the vlan id
*          *gip_ap            - the group ip
*          *sip_ap            - the source ip
*          port_info_p        - the port info
*          lport              - the logical port
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_AddPortHostIp(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    UI8_T                *host_src_ip,
    MLDSNP_OM_PortInfo_T *port_info_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeletePortHostIp
*------------------------------------------------------------------------------
* Purpose: This function delet the host ip from port
* INPUT  : vid                - the vlan id
*          *gip_ap            - the group ip
*          *sip_ap            - the source ip
*          port_info_p        - the port info
*          port_info_p        - the port info
*          lport              - the logical port
* OUTPUT : None
* RETURN : host ocunt on this port
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_DeletePortHostIp(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    UI8_T                *host_src_ip,
    MLDSNP_OM_PortInfo_T *port_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetFirstPortInfo
*------------------------------------------------------------------------------
* Purpose: This function get the first port info in the hisam entry
* INPUT  : vid                   - the vlan id
*          *gip_ap               - the group ip
*          *sip_ap               - the source ip
*
* OUTPUT : retrun_port_info_p - the port info
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetFirstPortInfo(
                                  UI16_T               vid,
                                  UI8_T                *gip_ap,
                                  UI8_T                *sip_ap,
                                  MLDSNP_OM_PortInfo_T *retrun_port_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextPortInfo
*------------------------------------------------------------------------------
* Purpose: This function get the next port info
* INPUT  : vid                   - the vlan id
*          *gip                  - the group ip
*          *sip                  - the source ip
*          *lport                - the input port number to get next port number
* OUTPUT : return_port_info_p    - the next group info
*                *lport          - the next port
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  return fail mean in this group entry has no port exist.
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextPortInfo(
                                 UI16_T vid,
                                 UI8_T                *gip_ap,
                                 UI8_T                *sip_ap,
                                 UI16_T               *lport,
                                 MLDSNP_OM_PortInfo_T *retrun_port_info_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRouterPortInfo
*------------------------------------------------------------------------------
* Purpose: This function get the router port info
* INPUT  : vid                       - the vlan id
*          r_port                    - the router port no
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : return_router_port_info_p - the router port info
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetRouterPortInfo(
                                   UI16_T                     vid,
                                   UI16_T                     r_port,
                                   MLDSNP_OM_RouterPortInfo_T *return_router_port_info_p);


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextRouterPortInfo
*------------------------------------------------------------------------------
* Purpose: This function get the router port info
* INPUT  : vid                       - the vlan id
*          r_port                    - the port number to get next
* OUTPUT : return_router_port_info_p - the router port info
*          *r_port                   - the next router port
* RETURN :TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextRouterPortInfo(
                                       UI16_T                     vid,
                                       UI16_T                     *r_port,
                                       MLDSNP_OM_RouterPortInfo_T *return_router_port_info_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_AddRouterPort
*------------------------------------------------------------------------------
* Purpose: This function add the router port into vlan
* INPUT  : vid                - the vlan id
*          *router_port_info_p- the router port info
*
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_AddRouterPort(
                               UI16_T                     vid,
                               MLDSNP_OM_RouterPortInfo_T *router_port_info_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteRouterPort
*------------------------------------------------------------------------------
* Purpose: This function remove the router port from vlan
* INPUT  : vid                - the vlan id
*          *router_port_info_p- the router port info
*
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteRouterPort(
                                  UI16_T                     vid,
                                  MLDSNP_OM_RouterPortInfo_T *router_port_info_p);

/*------------------------------------------------------------------------------
* Function :  MLDSNP_OM_UpdateRouterPortInfo
*------------------------------------------------------------------------------
* Purpose: This function update the router port info
* INPUT  : vid                - the vlan id
*          *router_port_info_p- the router port info
*
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_UpdateRouterPortInfo(
                                      UI16_T                     vid,
                                      MLDSNP_OM_RouterPortInfo_T *router_port_info_p);

/*------------------------------------------------------------------------------
* Function :  MLDSNP_OM_IsRouterPort
*------------------------------------------------------------------------------
* Purpose: This function check this port is router port
* INPUT  : vid - the vlan id to check
*              lport - the logical port to check
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_IsRouterPort(
                            UI16_T vid,
                            UI16_T lport);

/*------------------------------------------------------------------------------
* Function :  MLDSNP_OM_SetVlanInfo
*------------------------------------------------------------------------------
* Purpose: This function set the vlan info
* INPUT  :*add_vlan_info_p - the vlan info which will be update or insert to list
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetVlanInfo(
                            MLDSNP_OM_VlanInfo_T *add_vlan_info_p);
/*------------------------------------------------------------------------------
* Function :  MLDSNP_OM_GetVlanInfo
*------------------------------------------------------------------------------
* Purpose: This function get the vlan info
* INPUT  :vid - the vlan info which will be get
* OUTPUT : *vlan_info_p
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetVlanInfo(
                             UI16_T               vid,
                             MLDSNP_OM_VlanInfo_T *vlan_info_p);
/*------------------------------------------------------------------------------
* Function :  MLDSNP_OM_GetNextVlanInfo
*------------------------------------------------------------------------------
* Purpose: This function get the next vlan info
* INPUT  :*vid_p - the vlan info which will be get
* OUTPUT : *vlan_info_p
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextVlanInfo(
                                 UI16_T               *vid_p,
                                 MLDSNP_OM_VlanInfo_T *vlan_info_p);
/*------------------------------------------------------------------------------
* Function :  MLDSNP_OM_DeletetVlanInfo
*------------------------------------------------------------------------------
* Purpose: This function delete the vlan info
* INPUT  :*vlan_info_p.vids - the vlan info which will be deleted
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteVlanInfo(
                            MLDSNP_OM_VlanInfo_T *vlan_info_p);

#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set query guard status
 * INPUT   : status  - the enabled or diabled  status
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetQueryDropStatus(UI32_T lport, BOOL_T status);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get the query guard status
 * INPUT   : None
 * OUTPUT  : status  - the enabled or diabled status
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetQueryDropStatus(UI32_T lport, BOOL_T  *status);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get the query guard status
 * INPUT   : None
 * OUTPUT  :
 * RETURN  : TRUE  - query guard enble
 *           FALSE - query guard disable
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_IsQueryDropEnable(UI32_T lport);
#endif
#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set multicast data drop status
 * INPUT   : status  - the enabled or diabled  status
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetMulticastDataDropStatus(UI32_T lport, BOOL_T status);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get the multicast data drop status
 * INPUT   : None
 * OUTPUT  : status  - the enabled or diabled status
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetMulticastDataDropStatus(UI32_T lport, BOOL_T  *status);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get the multicast data drop status
 * INPUT   : None
 * OUTPUT  : status  - the enabled or diabled status
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_IsMulticastDataDropEnable(UI32_T lport);
#endif

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE )
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetPortMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : ifindex       - which port to get
 *           limit_per_sec - per second limit value
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetPortMldReportLimitPerSec(UI32_T lport, UI16_T limit_per_sec);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetPortMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : ifindex       - which port or vlan to get
 * OUTPUT  : limit_per_sec - per second limit value
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI16_T MLDSNP_OM_GetPortMldReportLimitPerSec(UI32_T lport);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_IsPortMldReportRcvdReachLimit
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to count mld report receceived value per second
 * INPUT   : ifindex       - which port to get
 *           limit_per_sec - per second limit value
 * OUTPUT  : None
 * RETURN  : TRUE  - reach limit
 *           FALSE - not reach limit
 * NOTE    : when call this function, it will add one.
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_IsPortMldReportRcvdReachLimit(UI32_T lport);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_ResetPortMldReportPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to reste mld report limit value per second
 * INPUT   :  None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
void MLDSNP_OM_ResetPortMldReportPerSec();
#endif

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE )
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetVlanMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : vid       - which vlan to get
 *           limit_per_sec - per second limit value
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetVlanMldReportLimitPerSec(UI32_T vid, UI16_T limit_per_sec);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetVlangmpReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : vid       - which vlan to get
 * OUTPUT  :
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI16_T MLDSNP_OM_GetVlanMldReportLimitPerSec(UI32_T vid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_IsPortMldReportRcvdReachLimit
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to count mld report receceived value per second
 * INPUT   : vid       - which vlan to test
 * OUTPUT  : None
 * RETURN  : TRUE  - reach limit
 *           FALSE - not reach limit
 * NOTE    : when call this function, it will add one.
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_IsVlanMldReportRcvdReachLimit(UI32_T vid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_ResetPortMldReportPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to reste mld report limit value per second
 * INPUT   :
 * OUTPUT  :
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
void MLDSNP_OM_ResetVlanMldReportPerSec();
#endif /*End of #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)*/

#if ( SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetMldFilterStatus
 *-------------------------------------------------------------------------
 * PURPOSE : Set Filter statutus
 * INPUT   : mldsnp_filter_status - status
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetMldFilterStatus(UI32_T mldsnp_filter_status);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_IsMldProfileExist
 *-------------------------------------------------------------------------
 * PURPOSE : Check this profile id is exist
 * INPUT   : pid  - profile id
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_IsMldProfileExist(UI32_T pid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_CreateMldProfileEntry
 *-------------------------------------------------------------------------
 * PURPOSE : Create new profile
 * INPUT   : pid  - profile id
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    : it won't check this pid is exist or not
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_CreateMldProfileEntry(UI32_T pid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_RemoveMldProfileFromAllPort
 *-------------------------------------------------------------------------
 * PURPOSE : unbind all port from pid
 * INPUT   : pid  - which pid to unbind
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_RemoveMldProfileFromAllPort(UI32_T pid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_DestroyMldProfileEntry
 *-------------------------------------------------------------------------
 * PURPOSE : Destroy profile entry
 * INPUT   : pid  - which pid to destroy
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_DestroyMldProfileEntry(UI32_T pid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetMldProfileAccessMode
 *-------------------------------------------------------------------------
 * PURPOSE : Set profile access mode
 * INPUT   : pid         - which pid to set
 *           access_mode - set which mode
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetMldProfileAccessMode(UI32_T pid, UI32_T access_mode);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_AddMldProfileGroup
 *-------------------------------------------------------------------------
 * PURPOSE : Add a group range to a profile
 * INPUT   : pid       - which pid to add
 *           start_mip - start ip address
 *           end_mip   - end ip address
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_AddMldProfileGroup(UI32_T pid, UI8_T *start_mip, UI8_T *end_mip);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_DeleteMldProfileGroup
 *-------------------------------------------------------------------------
 * PURPOSE : Delete mld range from a group
 * INPUT   : pid  - which pid to delete group range
 *           start_mip -  *           start_mip - start ip address
 *           end_mip   - end ip address
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    : the start_mip and end_mip shall match
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_DeleteMldProfileGroup(UI32_T pid, UI8_T *start_mip, UI8_T *end_mip);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_AddMldProfileToPort
 *-------------------------------------------------------------------------
 * PURPOSE : bind a profile to a port
 * INPUT   : lport  - which port to bind
 *           pid    - which pid
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_AddMldProfileToPort(UI32_T lport, UI32_T pid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_RemoveMldProfileFromPort
 *-------------------------------------------------------------------------
 * PURPOSE : Unbind profiles from a port
 * INPUT   : lport - which port to unbind
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_RemoveMldProfileFromPort(UI32_T lport);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetMldThrottlingInfo
 *-------------------------------------------------------------------------
 * PURPOSE : Get throttle information
 * INPUT   : lport   - which port to get
 * OUTPUT  : entry_p - throtle information
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetMldThrottlingInfo(UI32_T lport, MLDSNP_OM_Throttle_T *entry_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetPortMldProfileID
 *-------------------------------------------------------------------------
 * PURPOSE : Get port bind to which profle
 * INPUT   : lport - which port to get
 * OUTPUT  : pid_p - which pid
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetPortMldProfileID(UI32_T lport, UI32_T *pid_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetNextMldProfileGroupbyPid
 *-------------------------------------------------------------------------
 * PURPOSE : Get next profile group range by pid
 * INPUT   : pid   - which pid to get
 * OUTPUT  : start_mip_p - start ip address
 *           end_mip_p   - end ip address
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetNextMldProfileGroupbyPid(UI32_T pid, UI8_T *start_mip_p, UI8_T *end_mip_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetMldThrottlingActionToPort
 *-------------------------------------------------------------------------
 * PURPOSE : Set throttle action to a port
 * INPUT   : lport - which port to set action
 *           acction_mode - which action
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetMldThrottlingActionToPort(UI32_T lport, UI32_T action_mode);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetMldProfileAccessMode
 *-------------------------------------------------------------------------
 * PURPOSE : Get profile access mode
 * INPUT   : pid          - which pid to get
 * OUTPUT  : *access_mode - which access mode
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetMldProfileAccessMode(UI32_T pid, UI32_T *access_mode);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetMldFilterStatus
 *-------------------------------------------------------------------------
 * PURPOSE : Get filter status
 * INPUT   : None
 * OUTPUT  : *mldsnp_filter_status
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetMldFilterStatus(UI32_T *mldsnp_filter_status);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetMldThrottlingNumberToPort
 *-------------------------------------------------------------------------
 * PURPOSE : Set port throttle number
 * INPUT   : lport - which port to set
 *           throttling_number - throttle number
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetMldThrottlingNumberToPort(UI32_T lport, UI32_T throttling_number);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetNextMldProfileID
 *-------------------------------------------------------------------------
 * PURPOSE : Get next mld profile id which already created
 * INPUT   : *pid_p - current profile id
 * OUTPUT  : *pid_p - next profile id
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetNextMldProfileID(UI32_T *pid_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetNextMldProfile
 *-------------------------------------------------------------------------
 * PURPOSE : Get next profile information
 * INPUT   : *pid_p - current profile id
 * OUTPUT  : profile info - next profile information
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    : only for backdoor use
 *-------------------------------------------------------------------------
 */
MLDSNP_OM_ProfileInfo_T * MLDSNP_OM_GetNextMldProfile(UI32_T *pid_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetTotalProfileGroupRangeCount
 *-------------------------------------------------------------------------
 * PURPOSE : Get total profile group's range count
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    : for backdoor use
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_OM_GetTotalProfileGroupRangeCount();
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_IsProfileGroup
 *-------------------------------------------------------------------------
 * PURPOSE : Check this group is in this profile configured group range
 * INPUT   : pid    - which profile id
 *           *gip_ap- group address
 * OUTPUT  : *has_group - has this group or not
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_IsProfileGroup(UI32_T pid, UI8_T *gip_ap, BOOL_T *has_group);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetThrottleStatus
 *-------------------------------------------------------------------------
 * PURPOSE : Get throttle configure status
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : throllt status
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_OM_GetThrottleStatus(UI32_T lport);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetMLDThrottlingAction
 *-------------------------------------------------------------------------
 * PURPOSE : Get throttle action
 * INPUT   : lport - which port to get
 * OUTPUT  : action_mode - which action mode
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetMLDThrottlingAction(UI32_T lport, UI32_T *action_mode);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetPortMLDThrottlingNumber
 *-------------------------------------------------------------------------
 * PURPOSE : Get port throttle number
 * INPUT   : lport - which port to get
 * OUTPUT  : throttling_number - configured throttle number
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetPortMLDThrottlingNumber(UI32_T lport, UI32_T *throttling_number);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetPortDynamicGroupCount
 *-------------------------------------------------------------------------
 * PURPOSE : Get this port current learned group count
 * INPUT   : lport - which port to get
 * OUTPUT  : *current_count - current group learned on this port
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetPortDynamicGroupCount(UI32_T lport, UI32_T *current_count);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_PortDynamicGroupCountAddOne
 *-------------------------------------------------------------------------
 * PURPOSE : Increate this port learned gorup count
 * INPUT   : lport - which port to increase
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_PortDynamicGroupCountAddOne(UI32_T lport);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_PortDynamicGroupCountSubtractOne
 *-------------------------------------------------------------------------
 * PURPOSE : decrease this port learned gorup count
 * INPUT   : lport - which port to increase
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_PortDynamicGroupCountSubtractOne(UI32_T lport);
#endif
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_PortIncStat
 *-------------------------------------------------------------------------
 * PURPOSE : Increase port statistics
 * INPUT   : lport - which port to increase
 *           vid   - which vlan id
 *           type  - which statistics type
 *           inc   - increase or decrease
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_PortIncStat(UI32_T lport, UI32_T vid, MLDSNP_TYPE_Statistics_T type, BOOL_T inc);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_VlanIncStat
 *-------------------------------------------------------------------------
 * PURPOSE : Increase/decrease port statistics
 * INPUT   : vid   - which vlan id
 *           type  - which statistics type
 *           inc   - increase or decrease
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_VlanIncStat(UI32_T vid, MLDSNP_TYPE_Statistics_T type, BOOL_T inc);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_ClearInterfaceStatistics
 *-------------------------------------------------------------------------
 * PURPOSE : Clear all statistics on this port or vlan
 * INPUT   : ifindex - port or vlan id
 *           is_vlan - ifindex mean port or vlan id
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_ClearInterfaceStatistics(UI32_T ifindex, BOOL_T is_vlan);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetInterfaceStatistics
 *-------------------------------------------------------------------------
 * PURPOSE : Get interface statistics
 * INPUT   : ifindex - port or vlan id
 *           is_vlan - is vlan id or port
 * OUTPUT  : statistics_p - statistics
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetInterfaceStatistics(UI32_T ifindex, BOOL_T is_vlan, MLDSNP_OM_InfStat_T *statistics_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : MLDSNP_OM_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 * RETURN:
 *    TRUE  --  There is a response need to send.
 *    FALSE --  No response need to send.
 * NOTES:
 *    1.The size of ipcmsg_p.msg_buf must be large enough to carry any response
 *      messages.
 *------------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#endif/* End of mldsnp_om_H */


