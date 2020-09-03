/******************************************************************************
 * Filename: ospf6_mgr.h
 * File description:
 *        
 * Copyright (C) 2005-2007 unknown, Inc. All Rights Reserved.
 *        
 * author: steven.gao
 * Create Date: Tuesday, July 14, 2009 
 *        
 * Modify History
 *   Modifyer: 
 *   Date:     
 *   Description:
 *        
 *   Modifyer: 
 *   Date:     
 *   Description:
 *        
 * Version: 
 ******************************************************************************/
#ifndef _OSPF6_MGR_H
#define _OSPF6_MGR_H
/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "l_prefix.h"
#include "ospf6_type.h"

#define OSPF6_MGR_IPCMSG_TYPE_SIZE sizeof(union OSPF6_MGR_IPCMsg_Type_U)

#define OSPF6_MGR_MSG_HEADER_SIZE ((UI32_T)(&(((OSPF6_MGR_IPCMsg_T*)0)->data)))


#define OSPF6_MGR_GET_MSGBUF_SIZE(msg_data_type) \
    (OSPF6_MGR_IPCMSG_TYPE_SIZE + sizeof(msg_data_type))

#define OSPF6_MGR_GET_MSG_SIZE(field_name)                       \
            (OSPF6_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((OSPF6_MGR_IPCMsg_T *)0)->data.field_name))

#define OSPF6_VLINK_NAME_MAXLEN		16

typedef struct OSPF6_MGR_NBR_ENTRY_S
{
    /* indexed by {vr_id, tag, ifindex, nbr_rtr_id} */
    UI32_T      vr_id;
    char        tag[OSPF6_TAG_LEN];
    UI32_T      ifindex;
    UI32_T      nbr_rtr_id;

    UI32_T              instance_id;
    L_INET_AddrIp_T     nbr_ip_addr;

    UI32_T              nbr_options; 
    UI8_T               nbr_priority;   
    UI8_T               nbr_state;
    UI16_T              router_role;

    UI32_T              nbr_events;         /* ospfv3NbrEvents */
    UI32_T              nbr_ls_retrans;     /* ospfv3NbrLsRetransQLen */
    BOOL_T              nbr_hello_suppress; /* ospfv3NbrHelloSuppressed */
    UI32_T              nbr_ifid;
} OSPF6_MGR_NBR_ENTRY_T;


typedef struct OSPF6_MGR_VNBR_ENTRY_S
{
    /* indexed by {vr_id, tag, area_id, nbr_rtr_id} */
    UI32_T      vr_id;
    char        tag[OSPF6_TAG_LEN];
    struct pal_in4_addr     area_id;
    struct pal_in4_addr     nbr_rtr_id;

    char                name[OSPF6_VLINK_NAME_MAXLEN];
    UI32_T              nbr_if_index;
    UI32_T              instance_id;
    
    L_INET_AddrIp_T     nbr_ip_addr;

    UI32_T              nbr_options; 
    UI8_T               nbr_state;
    UI16_T              router_role;

    UI32_T              nbr_events;         /* ospfv3NbrEvents */
    UI32_T              nbr_ls_retrans;     /* ospfv3NbrLsRetransQLen */
    BOOL_T              nbr_hello_suppress; /* ospfv3NbrHelloSuppressed */
    UI32_T              nbr_ifid;
    UI32_T              nbr_priority;
} OSPF6_MGR_VNBR_ENTRY_T;



typedef struct OSPF6_MGR_LSA_HEADER_S
{
  /* LS age. */
  UI16_T ls_age;

  /* LS type. */
  UI16_T type;

  /* Link State ID. */
  struct pal_in4_addr id;

  /* Advertising Router. */
  struct pal_in4_addr adv_router;

  /* LS Sequence Number. */
  UI32_T ls_seqnum;

  /* Checksum. */
  UI16_T checksum;

  /* Length. */
  UI16_T length;
} OSPF6_MGR_LSA_HEADER_T;

typedef struct OSPF6_MGR_LSA_ENTRY_S
{
    UI32_T  vr_id;
    char    tag[OSPF6_TAG_LEN];
    int     type;
    union {
        struct {
            int ifindex;
            int instance_id; 
        } data; 
        struct pal_in4_addr area_id;
    } u;                        /* valid for AreaScopeLSA and LinkScopeLSA */
    struct pal_in4_addr adv_router;
    struct pal_in4_addr ls_id;    

    int     indexlen;

    UI32_T flags;
#define OSPF6_SHOW_LSDB_BY_SELF         (1 << 0)
#define OSPF6_SHOW_LSDB_BY_MAXAGE       (1 << 1)
#define OSPF6_SHOW_LSDB_BY_ADV_ROUTER   (1 << 2)
#define OSPF6_SHOW_LSDB_BY_LSID         (1 << 3)
#define OSPF6_SHOW_LSDB_BY_TYPE         (1 << 4)
#define OSPF6_SHOW_LSDB_BY_TAG          (1 << 5)

    struct pal_in4_addr router_id;    

    UI32_T      type_known;

    /* you can cast advertisement to OSPF6_MGR_LSA_HEADER_T for the detail */
    UI8_T       advertisement[OSPF6_TYPE_LSDB_ADVERTISE_SIZE];       /* be careful about the max length */
}OSPF6_MGR_LSA_ENTRY_T;

typedef struct OSPF6_MGR_OSPF_AREA_S
{
    UI8_T auth_type;
    UI8_T mode;
    int h;
    int m;
    int s;
    int us;
    UI8_T area_default;
    UI8_T backbone_b;
    UI8_T area_type;
    UI8_T summary_flag;
    UI8_T area_active;
    UI32_T spf_calc_count;
    UI8_T  active_if_count;
    UI32_T if_count;
    UI32_T full_nbr_count;
    UI32_T full_virt_nbr_count;
    UI32_T lsdb_count_all;
    UI32_T lsdb_checksum_all;
    UI8_T  shortcut_b;
}OSPF6_MGR_OSPF_AREA_T;

typedef struct OSPF6_MGR_OSPF_ENTRY_S
{
    UI32_T  vr_id;
    char    tag[OSPF6_TAG_LEN];

    BOOL_T  is_first;
    struct pal_in4_addr router_id;    

    /* Show Uptime. */
    int         day;
    int         hour;
    int         minute;

    UI32_T      spf_delay;          /* SPF delay time. */
    UI32_T      spf_holdtime;       /* SPF hold time. */
    UI32_T      default_metric;

    UI8_T   is_abr;
    UI8_T   abr_type;
    UI8_T   is_asbr;
    UI16_T  dd_count_in;
    UI16_T  dd_count_out;
    UI16_T  max_dd;

    UI32_T  external_lsa_num;
    UI32_T  checksum;

    UI32_T  asscope_lsa_num;
    UI32_T  self_lsa_count;
    UI32_T  rx_lsa_count;

    UI32_T  count_area;
}OSPF6_MGR_OSPF_ENTRY_T;

typedef struct OSPF6_MGR_NH_BORDER_ROUTER_S
{
    UI32_T flags;    
    struct pal_in4_addr nbr_id; 
    struct pal_in4_addr if_id;
    UI8_T  ifp_name[15];
    UI8_T  transit_b;
    struct pal_in4_addr area_id;    
} OSPF6_MGR_NH_BORDER_ROUTER_T;



/***************************************************
 **    ospf6_mgr ipc request command definitions    **
 ***************************************************
 */
enum
{
    /* system management message */
    OSPF6_MGR_IPC_INTERFACEADD,
    OSPF6_MGR_IPC_INTERFACEDELETE,
    OSPF6_MGR_IPC_IPADDRESSADD,
    OSPF6_MGR_IPC_IPADDRESSDELETE,
    OSPF6_MGR_IPC_INTERFACEUP,
    OSPF6_MGR_IPC_INTERFACEDOWN,

    /* router ipv6 ospf */
    OSPF6_MGR_IPC_ROUTEROSPFSET,
    OSPF6_MGR_IPC_ROUTEROSPFUNSET,

    /* interface management message */
    OSPF6_MGR_IPC_IFPARAM_CREATE,
    OSPF6_MGR_IPC_IFPARAM_DESTROY,

    OSPF6_MGR_IPC_IFPRIORITYSET,
    OSPF6_MGR_IPC_IFPRIORITYUNSET,
    OSPF6_MGR_IPC_IFCOSTSET,
    OSPF6_MGR_IPC_IFCOSTUNSET,
    OSPF6_MGR_IPC_IFDEADINTERVALSET,
    OSPF6_MGR_IPC_IFDEADINTERVALUNSET,
    OSPF6_MGR_IPC_IFHELLOINTERVALSET,
    OSPF6_MGR_IPC_IFHELLOINTERVALUNSET,
    OSPF6_MGR_IPC_IFRETRANSMITINTERVALSET,
    OSPF6_MGR_IPC_IFRETRANSMITINTERVALUNSET,
    OSPF6_MGR_IPC_IFTRANSMITDELAYSET,
    OSPF6_MGR_IPC_IFTRANSMITDELAYUNSET,
    OSPF6_MGR_IPC_IFROUTEROSPFSET,
    OSPF6_MGR_IPC_IFROUTEROSPFUNSET,

    /* OSPFv3 configuration message */
    OSPF6_MGR_IPC_ABRTYPE_SET, 
    OSPF6_MGR_IPC_ABRTYPE_UNSET, 
    OSPF6_MGR_IPC_AREADEFAULTCOSTSET,
    OSPF6_MGR_IPC_AREADEFAULTCOSTUNSET,
    OSPF6_MGR_IPC_AREARANGE_SET,
    OSPF6_MGR_IPC_AREARANGENOADVERTISESET,
    OSPF6_MGR_IPC_AREARANGE_UNSET,

    OSPF6_MGR_IPC_AREASTUBSET,
    OSPF6_MGR_IPC_AREASTUBUNSET,
    OSPF6_MGR_IPC_AREASTUBNOSUMMARYSET,
    OSPF6_MGR_IPC_AREASTUBNOSUMMARYUNSET,
    OSPF6_MGR_IPC_AREAVLINKSET,
    OSPF6_MGR_IPC_AREAVLINKUNSET,
    OSPF6_MGR_IPC_DEFAULTMETRICSET,
    OSPF6_MGR_IPC_DEFAULTMETRICUNSET,
    OSPF6_MGR_IPC_PASSIVEINTERFACESET,
    OSPF6_MGR_IPC_PASSIVEINTERFACEUNSET,

    OSPF6_MGR_IPC_MAX_CONCURRENT_DD_SET, 
    OSPF6_MGR_IPC_MAX_CONCURRENT_DD_UNSET, 

    OSPF6_MGR_IPC_REDISTRIBUTE_PROTOSET,
    OSPF6_MGR_IPC_REDISTRIBUTE_PROTOUNSET,
    OSPF6_MGR_IPC_REDISTRIBUTE_METRICSET,
    OSPF6_MGR_IPC_REDISTRIBUTE_METRICUNSET,
    OSPF6_MGR_IPC_REDISTRIBUTE_METRICTYPESET,
    OSPF6_MGR_IPC_REDISTRIBUTE_METRICTYPEUNSET,
    OSPF6_MGR_IPC_REDISTRIBUTE_ROUTEMAPSET,
    OSPF6_MGR_IPC_REDISTRIBUTE_ROUTEMAPUNSET,
    OSPF6_MGR_IPC_GET_MULTIPROCREDISTRIBUTETABLE,
    OSPF6_MGR_IPC_GETNEXT_MULTIPROCREDISTRIBUTETABLE,

    OSPF6_MGR_IPC_ROUTERIDSET,
    OSPF6_MGR_IPC_ROUTERIDUNSET,
    OSPF6_MGR_IPC_DELAY_TIMERSET,
    OSPF6_MGR_IPC_HOLD_TIMERSET,
    OSPF6_MGR_IPC_TIMERUNSET,

    OSPF6_MGR_IPC_DISPLAY_SINGLE_SET, 
    OSPF6_MGR_IPC_DISPLAY_SINGLE_UNSET, 

    OSPF6_MGR_IPC_CLEAR_OSPF6_PROCESS,

    /* ospf6 Get/GetNext */
    OSPF6_MGR_IPC_GET_OSPF_ENTRY,
    OSPF6_MGR_IPC_GETNEXT_OSPF_ENTRY,
    OSPF6_MGR_IPC_GET_AREA,
    OSPF6_MGR_IPC_GETNEXT_AREA,
    OSPF6_MGR_IPC_GETAREARANGE,
    OSPF6_MGR_IPC_GETNEXTAREARANGE,
    OSPF6_MGR_IPC_GETPASSIVEIF,
    OSPF6_MGR_IPC_GETNEXTPASSIVEIF,

    OSPF6_MGR_IPC_GETNEXT_LINKLSDB_ENTRY,
    OSPF6_MGR_IPC_GETNEXT_AREALSDB_ENTRY,
    OSPF6_MGR_IPC_GETNEXT_ASLSDB_ENTRY,

    OSPF6_MGR_IPC_GET_INTERFACE_ENTRY, 
    OSPF6_MGR_IPC_GETNEXT_INTERFACE_ENTRY, 
    OSPF6_MGR_IPC_GET_MULTIPROCIFTABLE,
    OSPF6_MGR_IPC_GETNEXT_MULTIPROCIFTABLE,

    OSPF6_MGR_IPC_GET_NBR_ENTRY,
    OSPF6_MGR_IPC_GETNEXT_NBR_ENTRY,
    OSPF6_MGR_IPC_GET_MULTIPROCVIRTNBR,
    OSPF6_MGR_IPC_GETNEXT_MULTIPROCVIRTNBR,

    OSPF6_MGR_IPC_GETNEXT_ROUTE,
    OSPF6_MGR_IPC_GETNEXT_TOPO, 
    OSPF6_MGR_IPC_GET_MULTIPROCVIRTUALLINKENTRY,
    OSPF6_MGR_IPC_GETNEXT_MULTIPROCVIRTUALLINKENTRY,

    OSPF6_MGR_IPC_GET_IFPARAM_ENTRY,
    OSPF6_MGR_IPC_GETNEXT_IFPARAM_ENTRY,
#if 0
    /*
     *
     */
    OSPF6_MGR_IPC_IFAREAIDSET,
    OSPF6_MGR_IPC_IFAREAIDUNSET,
    OSPF6_MGR_IPC_NETWORKSET,
    OSPF6_MGR_IPC_NETWORKUNSET,
    OSPF6_MGR_IPC_AREARANGESTATUSSET,
    OSPF6_MGR_IPC_AREARANGEEFFECTSET,
    OSPF6_MGR_IPC_GETNEXTPROCESSSTATUS,
    OSPF6_MGR_IPC_GETNEXTAREAPARA,
    OSPF6_MGR_IPC_GETNETWORKENTRY,
    OSPF6_MGR_IPC_GETNEXTNETWORKENTRY,
    OSPF6_MGR_IPC_AREASUMMARYSET,
    OSPF6_MGR_IPC_GETSTUBAREAENTRY,
    OSPF6_MGR_IPC_GETNEXTSTUBAREAENTRY,
    OSPF6_MGR_IPC_STUBAREAMETRICSET,
    OSPF6_MGR_IPC_STUBAREASTATUSSET,
    OSPF6_MGR_IPC_SUMMARY_ADDRSET,
    OSPF6_MGR_IPC_SUMMAYR_ADDRUNSET,
    OSPF6_MGR_IPC_GET_MULTIPROCSUMMARYADDRTABLE,
    OSPF6_MGR_IPC_GETNEXT_MULTIPROCSUMMARYADDRTABLE,
    OSPF6_MGR_IPC_DEFAULTINFO_METRICSET,
    OSPF6_MGR_IPC_DEFAULTINFO_METRICUNSET,
    OSPF6_MGR_IPC_DEFAULTINFO_METRICTYPESET,
    OSPF6_MGR_IPC_DEFAULTINFO_METRICTYPEUNSET,
    OSPF6_MGR_IPC_DEFAULTINFO_ROUTEMAPSET,
    OSPF6_MGR_IPC_DEFAULTINFO_ROUTEMAPUNSET,
    OSPF6_MGR_IPC_DEFAULTINFO_ALWAYSSET,
    OSPF6_MGR_IPC_DEFAULTINFO_ALWWAYSUNSET,
    OSPF6_MGR_IPC_DEFAULTINFO_SET,
    OSPF6_MGR_IPC_DEFAULTINFO_UNSET,
    OSPF6_MGR_IPC_GETOSPFINTERFACEENTRY,
    OSPF6_MGR_IPC_GETNEXT_OSPF_DATABASE_ROUTER,
    OSPF6_MGR_IPC_CLEAR_OSPF_PROCESS,
    OSPF6_MGR_IPC_GET_ABR_BORDER_ROUTER_ENTRY,
    OSPF6_MGR_IPC_GET_ASBR_BORDER_ROUTER_ENTRY,
    OSPF6_MGR_IPC_GET_MULTIPROCVIRTIFMD5KEY,
    OSPF6_MGR_IPC_GETNEXT_MULTIPROCVIRTIFMD5KEY,
    OSPF6_MGR_IPC_GET_IFPARAM_ENTRY,
    OSPF6_MGR_IPC_GETNEXT_IFPARAM_ENTRY,
    OSPF6_MGR_IPC_SET_IFPARAM_ENTRY,
    OSPF6_MGR_IPC_SET_AREALIMIT,
    OSPF6_MGR_IPC_GET_MULTI_PROCESS_System_ENTRY,
    OSPF6_MGR_IPC_GETNEXT_MULTI_PROCESS_System_ENTRY,
    OSPF6_MGR_IPC_GET_MULTI_PROCESS_LSDB_ENTRY,
    OSPF6_MGR_IPC_GETNEXT_MULTI_PROCESS_LSDB_ENTRY,
    OSPF6_MGR_IPC_GET_MULTI_PROCESS_EXT_LSDB_ENTRY,
    OSPF6_MGR_IPC_GETNEXT_MULTI_PROCESS_EXT_LSDB_ENTRY,
#endif
};


/*****************************************
 **      ofpf_mgr ipc msg structure      **
 *****************************************
 */

typedef struct OSPF6_MGR_IPCMsg_S
{
    union OSPF6_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request command */
        BOOL_T result_bool;      /*respond bool return*/
        UI32_T result_ui32;      /*respond ui32 return*/ 
        SYS_TYPE_Get_Running_Cfg_T         result_running_cfg; /* For running config API */
    } type;
    
    union
    {
        OSPF6_MGR_OSPF_ENTRY_T          ospf6_entry;
        OSPF6_TYPE_Area_T               arg_area;
        OSPF6_TYPE_Area_Range_T         arg_area_range;
        OSPF6_TYPE_Multi_Proc_Redist_T  arg_redistribute;
        OSPF6_TYPE_Interface_T          arg_interface;       
        OSPF6_TYPE_IfParam_T            arg_ifparam;
        OSPF6_TYPE_Route_T              arg_route;
        OSPF6_MGR_NBR_ENTRY_T           ospf6_neighbor_entry;
        OSPF6_MGR_VNBR_ENTRY_T          ospf6_virt_neighbor;
        OSPF6_TYPE_Vlink_T              ospf6_virtual_link_entry;
        OSPF6_MGR_LSA_ENTRY_T           ospf6_lsa_entry;

        struct    
        {
            UI32_T          arg1;
            UI32_T          arg2;
            UI32_T          arg3;
            L_INET_AddrIp_T arg4;
            UI32_T          arg5;
        } arg_address;

        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
        } arg_2;
        
        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
        } arg_3;

        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
        } arg_4;
        
        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
            UI32_T   arg5;
        } arg_5;
        
        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
            UI32_T   arg5;
            UI32_T   arg6;
        } arg_6;
       
       struct    
       {
           UI32_T   arg1;
           UI32_T   arg2;
           UI32_T   arg3;
           UI32_T   arg4;
           UI32_T   arg5;
           UI32_T   arg6;
           UI32_T   arg7;
       } arg_7;
       
       struct    
       {
           UI32_T   arg1;
           char     arg2[OSPF6_TAG_LEN];
       } arg_tag_0;

        struct    
        {
            UI32_T   arg1;
            char     arg2[OSPF6_TAG_LEN];
            UI32_T   arg3;
        } arg_tag_1;
        
        struct    
        {
            UI32_T   arg1;
            char     arg2[OSPF6_TAG_LEN];
            UI32_T   arg3;
            UI32_T   arg4;
        } arg_tag_2;
        
        struct    
        {
            UI32_T   arg1;
            char     arg2[OSPF6_TAG_LEN];
            UI32_T   arg3;
            UI32_T   arg4;
            UI32_T   arg5;
        } arg_tag_3;

        struct    
        {
            UI32_T   arg1;
            char     arg2[OSPF6_TAG_LEN];
            UI32_T   arg3;
            UI32_T   arg4;
            UI32_T   arg5;
            UI32_T   arg6;
        } arg_tag_4;

        struct    
        {
            UI32_T   arg1;
            char     arg2[OSPF6_TAG_LEN];
            UI32_T   arg3;
            UI32_T   arg4;
            UI32_T   arg5;
            UI32_T   arg6;
            UI32_T   arg7;
        } arg_tag_5;

        struct    
        {
            UI32_T   arg1;
            char     arg2[OSPF6_TAG_LEN];
            UI32_T   arg3;
            char     arg4[OSPF6_ROUTEMAP_LEN];
        } arg_tag_word_1;
    } data;
} OSPF6_MGR_IPCMsg_T;


void OSPF6_MGR_Init(void);


void OSPF6_MGR_SetTransitionMode(void);
void OSPF6_MGR_EnterTransitionMode(void);
void OSPF6_MGR_EnterMasterMode(void);
void OSPF6_MGR_EnterSlaveMode(void);

#ifdef OSPF6_UNIT_TEST

UI32_T OSPF6_MGR_SignalL3IfCreate(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T mtu, UI32_T bandwidth, UI32_T if_flags);
UI32_T OSPF6_MGR_SignalL3IfDestroy(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);
UI32_T OSPF6_MGR_SignalL3IfRifCreate(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, L_INET_AddrIp_T *addr, UI32_T primary);
UI32_T OSPF6_MGR_SignalL3IfRifDestroy(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, L_INET_AddrIp_T *addr);
UI32_T OSPF6_MGR_SignalL3IfUp(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);
UI32_T OSPF6_MGR_SignalL3IfDown(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);


UI32_T OSPF6_MGR_IfParamCreate(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id);
UI32_T OSPF6_MGR_IfParamDestroy(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id);
UI32_T OSPF6_MGR_IfPrioritySet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T priority);
UI32_T OSPF6_MGR_IfPriorityUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag);
UI32_T OSPF6_MGR_IfCostSet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T cost);
UI32_T OSPF6_MGR_IfCostUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag);
UI32_T OSPF6_MGR_IfDeadIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T interval);
UI32_T OSPF6_MGR_IfDeadIntervalUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag);
UI32_T OSPF6_MGR_IfHelloIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T interval);
UI32_T OSPF6_MGR_IfHelloIntervalUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag);
UI32_T OSPF6_MGR_IfRetransmitIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T interval);
UI32_T OSPF6_MGR_IfRetransmitIntervalUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag);
UI32_T OSPF6_MGR_IfTransmitDelaySet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T delay);
UI32_T OSPF6_MGR_IfTransmitDelayUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag);
UI32_T OSPF6_MGR_IfRouterSet(UI32_T vr_id, char * tag, UI32_T ifindex, UI32_T aid, UI32_T format, UI32_T id);
UI32_T OSPF6_MGR_IfRouterUnset(UI32_T vr_id, char * tag, UI32_T ifindex, UI32_T aid, UI32_T id, UI32_T check_flag);


UI32_T OSPF6_MGR_RouterOspfSet(UI32_T vr_id, char * tag);
UI32_T OSPF6_MGR_RouterOspfUnset(UI32_T vr_id, char * tag);
UI32_T OSPF6_MGR_ABRTypeSet (UI32_T vr_id, char * tag, UI32_T type);
UI32_T OSPF6_MGR_ABRTypeUnset (UI32_T vr_id, char * tag);
UI32_T OSPF6_MGR_AreaDefaultCostSet(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format, UI32_T cost);
UI32_T OSPF6_MGR_AreaDefaultCostUnset(UI32_T vr_id, char * tag, UI32_T area_id);
UI32_T OSPF6_MGR_AreaRangeSet(OSPF6_TYPE_Area_Range_T * range) ;
UI32_T OSPF6_MGR_AreaRangeUnset(OSPF6_TYPE_Area_Range_T * range) ;
UI32_T OSPF6_MGR_AreaStubSet(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format);
UI32_T OSPF6_MGR_AreaStubUnset(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format);
UI32_T OSPF6_MGR_AreaStubNoSummarySet(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format);
UI32_T OSPF6_MGR_AreaStubNoSummaryUnset(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format);
UI32_T OSPF6_MGR_AreaVirtualLinkSet(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format, UI32_T peer, UI32_T type, UI32_T value);
UI32_T OSPF6_MGR_AreaVirtualLinkUnset(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T peer, UI32_T type);
UI32_T OSPF6_MGR_DefaultMetricSet(UI32_T vr_id, char * tag, UI32_T metric);
UI32_T OSPF6_MGR_DefaultMetricUnset(UI32_T vr_id, char * tag);
UI32_T OSPF6_MGR_PassiveIfSet(UI32_T vr_id, char * tag, UI32_T ifindex);
UI32_T OSPF6_MGR_PassiveIfUnset(UI32_T vr_id, char * tag, UI32_T ifindex);
UI32_T OSPF6_MGR_PassiveIfGet(UI32_T vr_id, char * tag, UI32_T ifindex);
UI32_T OSPF6_MGR_PassiveIfGetNext(UI32_T vr_id, char * tag, UI32_T * ifindex);
UI32_T OSPF6_MGR_ConcurrentDDSet (UI32_T vr_id, char * tag, UI32_T number);
UI32_T OSPF6_MGR_ConcurrentDDUnset (UI32_T vr_id, char * tag);

UI32_T OSPF6_MGR_RedistributeProtoSet(UI32_T vr_id, char * tag, UI32_T proto);
UI32_T OSPF6_MGR_RedistributeProtoUnset(UI32_T vr_id, char * tag, UI32_T proto);
UI32_T OSPF6_MGR_RedistributeMetricTypeSet(UI32_T vr_id, char * tag, UI32_T proto, UI32_T metric_type);
UI32_T OSPF6_MGR_RedistributeMetricTypeUnset(UI32_T vr_id, char * tag, UI32_T proto);
UI32_T OSPF6_MGR_RedistributeMetricSet(UI32_T vr_id, char * tag, UI32_T proto, UI32_T metric);
UI32_T OSPF6_MGR_RedistributeMetricUnset(UI32_T vr_id, char * tag, UI32_T proto);
UI32_T OSPF6_MGR_RedistributeRoutemapSet(UI32_T vr_id, char * tag, UI32_T proto, char *route_map);
UI32_T OSPF6_MGR_RedistributeRoutemapUnset(UI32_T vr_id, char * tag, UI32_T proto);
UI32_T OSPF6_MGR_RedistributeEntry_Get(OSPF6_TYPE_Multi_Proc_Redist_T * data);
UI32_T OSPF6_MGR_RedistributeEntry_GetNext(OSPF6_TYPE_Multi_Proc_Redist_T *data);

UI32_T OSPF6_MGR_RouterIdSet(UI32_T vr_id, char * tag, UI32_T router_id);
UI32_T OSPF6_MGR_RouterIdUnset(UI32_T vr_id, char * tag);
UI32_T OSPF6_MGR_DelayTimerSet(UI32_T vr_id, char * tag, UI32_T delay);
UI32_T OSPF6_MGR_HoldTimerSet(UI32_T vr_id, char * tag, UI32_T hold);
UI32_T OSPF6_MGR_TimerUnset(UI32_T vr_id, char * tag);



UI32_T OSPF6_MGR_Process_Get(OSPF6_MGR_OSPF_ENTRY_T *ospf6);
UI32_T OSPF6_MGR_Process_GetNext(OSPF6_MGR_OSPF_ENTRY_T *ospf6);

UI32_T OSPF6_MGR_Area_Get(OSPF6_TYPE_Area_T *entry);
UI32_T OSPF6_MGR_Area_GetNext(OSPF6_TYPE_Area_T *entry);

UI32_T OSPF6_MGR_AreaRange_Get(OSPF6_TYPE_Area_Range_T *entry);
UI32_T OSPF6_MGR_AreaRange_GetNext(OSPF6_TYPE_Area_Range_T *entry);

UI32_T OSPF6_MGR_LinkScopeLSA_GetNext (OSPF6_MGR_LSA_ENTRY_T *lsa_entry_p);
UI32_T OSPF6_MGR_AreaScopeLSA_GetNext (OSPF6_MGR_LSA_ENTRY_T *lsa_entry_p);
UI32_T OSPF6_MGR_ASScopeLSA_GetNext (OSPF6_MGR_LSA_ENTRY_T *lsa_entry_p);

UI32_T OSPF6_MGR_Interface_Get(OSPF6_TYPE_Interface_T *entry);
UI32_T OSPF6_MGR_Interface_GetNext(OSPF6_TYPE_Interface_T *entry);

UI32_T OSPF6_MGR_IfParam_Get(OSPF6_TYPE_IfParam_T *entry);
UI32_T OSPF6_MGR_IfParam_GetNext(OSPF6_TYPE_IfParam_T *entry);

UI32_T OSPF6_MGR_Neighbor_Get(OSPF6_MGR_NBR_ENTRY_T *nbr_entry_p);
UI32_T OSPF6_MGR_Neighbor_GetNext(OSPF6_MGR_NBR_ENTRY_T *nbr_entry_p);

UI32_T OSPF6_MGR_VirtNeighbor_Get(OSPF6_MGR_VNBR_ENTRY_T *nbr_entry_p);
UI32_T OSPF6_MGR_VirtNeighbor_GetNext(OSPF6_MGR_VNBR_ENTRY_T *nbr_entry_p);

UI32_T OSPF6_MGR_Route_GetNext(OSPF6_TYPE_Route_T *entry);

UI32_T OSPF6_MGR_VirtualLink_Get(OSPF6_TYPE_Vlink_T *entry);
UI32_T OSPF6_MGR_VirtualLink_GetNext(OSPF6_TYPE_Vlink_T *entry);

#endif /* OSPF6_UNIT_TEST */


BOOL_T OSPF6_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *sysfun_msg_p);



/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */
#endif /*_OSPF6_MGR_H*/

