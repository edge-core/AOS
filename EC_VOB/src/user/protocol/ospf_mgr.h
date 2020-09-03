/* MODULE NAME:  ospf_mgr.h
 * PURPOSE:
 *     This module provides APIs for OSPF CSC to use.
 *
 * NOTES:
 *     None.
 *
 * HISTORY
 *    11/27/2008 - Lin.Li, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

#ifndef OSPF_MGR_H
#define OSPF_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "l_prefix.h"
#include "ospf_type.h"

#define OSPF_AUTH_SIMPLE_SIZE           8
#define OSPF_AUTH_MD5_SIZE              16

#define OSPF_MGR_IPCMSG_TYPE_SIZE sizeof(union OSPF_MGR_IPCMsg_Type_U)

#define OSPF_MGR_MSG_HEADER_SIZE ((UI32_T)(&(((OSPF_MGR_IPCMsg_T*)0)->data)))


#define OSPF_MGR_GET_MSGBUF_SIZE(msg_data_type) \
    (OSPF_MGR_IPCMSG_TYPE_SIZE + sizeof(msg_data_type))

#define OSPF_MGR_GET_MSG_SIZE(field_name)                       \
            (OSPF_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((OSPF_MGR_IPCMsg_T *)0)->data.field_name))

typedef struct OSPF_MGR_NBR_ENTRY_S
{
    UI32_T vr_id;
    UI32_T proc_id;
    UI32_T ospf_nbr_rtr_id;
    UI32_T ospf_nbr_ip_addr;
    UI8_T  ospf_nbr_state;
    UI8_T  ospf_nbr_priority;
    UI32_T router_role;
    UI32_T ifindex;
    struct pal_in4_addr area_id;
    struct pal_in4_addr nbr_id;
    UI32_T virt_nbr_b;
    UI8_T  ifname[20];
}OSPF_MGR_NBR_ENTRY_T;

typedef struct OSPF_MGR_LSA_HEADER_S
{
  /* LS age. */
  UI16_T ls_age;

  /* Options. */
  UI8_T options;

  /* LS type. */
  UI8_T type;

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

}OSPF_MGR_LSA_HEADER_T;

#define OSPF_MGR_LSA_LINK_NUMBER 5
typedef struct OSPF_MGR_LINK_S
{
  /* Link ID. */
  struct pal_in4_addr id;

  /* Link Data. */
  struct pal_in4_addr data;

  /* Link Type. */
  UI8_T type;

  /* # TOS. */
  UI8_T num_tos;

  /* TOS 0 Metric. */
  UI16_T metric;
}OSPF_MGR_LINK_T;

typedef struct OSPF_MGR_LSA_ENTRY_S
{
    UI32_T vr_id;
    UI32_T proc_id;
    struct pal_in4_addr area_id;
    UI32_T  last;
    UI32_T type;
    void * lsa_entry_p;
    UI32_T flags;
#define OSPF_SHOW_DATABASE_SELF     (1 << 0)
#define OSPF_SHOW_DATABASE_MAXAGE   (1 << 1)
#define OSPF_SHOW_DATABASE_ADV_ROUTER   (1 << 2)
#define OSPF_SHOW_DATABASE_ID       (1 << 3)
#define OSPF_GET_DATABASE_ID        (1 << 4)

    struct pal_in4_addr router_id;
    UI32_T show_brief_b;
    UI32_T show_flags;
#define OSPF_SHOW_DATABASE_HEADER_PROC      (1 << 0)
#define OSPF_SHOW_DATABASE_HEADER_TYPE      (1 << 1)
#define OSPF_SHOW_DATABASE_HEADER_BRIEF     (1 << 2)

    struct pal_in4_addr adv_router;
    struct pal_in4_addr link_id;
    OSPF_MGR_LSA_HEADER_T header;
    UI8_T bits;
    void  *lsa_position;
    void  *lsa_lim;
    struct timeval tv_update;
    UI32_T first_lsa_entry;
    UI32_T network_mask_len;
    UI32_T summary_metric;
    UI32_T e_bit;
    UI32_T tag;
    struct pal_in4_addr next_hop;
    UI32_T links;
    UI32_T area_type;
    union
    {
        struct pal_in4_addr neighbor_id[OSPF_MGR_LSA_LINK_NUMBER * 3];
        OSPF_MGR_LINK_T  link[OSPF_MGR_LSA_LINK_NUMBER];
    }data;
}OSPF_MGR_LSA_ENTRY_T;
typedef struct OSPF_MGR_OSPF_AREA_S
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
    /* NSSA TranslatorState. */
    UI8_T translator_state;
#define OSPF_NSSA_TRANSLATOR_DISABLED   0
#define OSPF_NSSA_TRANSLATOR_ENABLED    1
#define OSPF_NSSA_TRANSLATOR_ELECTED    2
}OSPF_MGR_OSPF_AREA_T;

typedef struct OSPF_MGR_OSPF_ENTRY_S
{
    UI32_T vr_id;
    UI32_T proc_id;
    struct pal_in4_addr area_id;
    UI32_T show_flags;
#define OSPF_SHOW_OSPF_HEADER_PROC      (1 << 0)
#define OSPF_GET_OSPF_ONE_PROC          (1 << 1)
    struct pal_in4_addr router_id;
    int day;
    int hour;
    int minute;
    UI8_T compatible_rfc1583;
    UI8_T is_abr;
    UI8_T abr_type;
    UI8_T is_asbr;
    UI32_T spf_delay;          /* SPF delay time. */
    UI32_T spf_holdtime;       /* SPF hold time. */
    UI16_T interval;
    UI16_T dd_count_in;
    UI16_T dd_count_out;
    UI16_T max_dd;
    UI32_T external_lsa_num;
    UI32_T checksum;
    UI8_T  overflow_lsdb_limit_b;
    UI32_T overflow_lsdb_limit;
    UI8_T  overflow_lsdb_already;
    UI32_T lsa_originate_count;
    UI32_T rx_lsa_count;
    UI32_T count_area;
    UI32_T indexlen;
    OSPF_MGR_OSPF_AREA_T area;
}OSPF_MGR_OSPF_ENTRY_T;

typedef struct OSPF_MGR_NH_BORDER_ROUTER_S
{
    UI32_T flags;
    struct pal_in4_addr nbr_id;
    struct pal_in4_addr if_id;
    UI8_T  ifp_name[15];
    UI8_T  transit_b;
    struct pal_in4_addr area_id;
}OSPF_MGR_NH_BORDER_ROUTER_T;

typedef struct OSPF_MGR_ASBR_BORDER_ROUTER_ENTRY_S
{
    UI32_T number;
    UI8_T  path_type;
    UI8_T  prefix[4];
    UI8_T  path_cost;
    UI8_T  path_flags;
    UI8_T  count;
    OSPF_MGR_NH_BORDER_ROUTER_T next_hop[1];
}OSPF_MGR_ASBR_BORDER_ROUTER_ENTRY_T;

typedef struct OSPF_MGR_ABR_BORDER_ROUTER_ENTRY_S
{
    UI32_T number;
    struct pal_in4_addr id;
    UI32_T distance;
    UI32_T count;
    OSPF_MGR_NH_BORDER_ROUTER_T next_hop[1];
}OSPF_MGR_ABR_BORDER_ROUTER_ENTRY_T;

/***************************************************
 **    ospf_mgr ipc request command definitions    **
 ***************************************************
 */
enum
{
    OSPF_MGR_IPC_ROUTEROSPFSET,
    OSPF_MGR_IPC_ROUTEROSPFUNSET,
    OSPF_MGR_IPC_INTERFACEADD,
    OSPF_MGR_IPC_INTERFACEDELETE,
    OSPF_MGR_IPC_IPADDRESSADD,
    OSPF_MGR_IPC_IPADDRESSDELETE,
    OSPF_MGR_IPC_IFAUTHENTICATIONTYPESET,
    OSPF_MGR_IPC_IFAUTHENTICATIONTYPEUNSET,
    OSPF_MGR_IPC_IFAUTHENTICATIONKEYSET,
    OSPF_MGR_IPC_IFAUTHENTICATIONKEYUNSET,
    OSPF_MGR_IPC_IFMESSAGEDIGESTKEYSET,
    OSPF_MGR_IPC_IFMESSAGEDIGESTKEYUNSET,
    OSPF_MGR_IPC_IFPRIORITYSET,
    OSPF_MGR_IPC_IFPRIORITYUNSET,
    OSPF_MGR_IPC_IFCOSTSET,
    OSPF_MGR_IPC_IFCOSTUNSET,
    OSPF_MGR_IPC_IFMTUSET,
    OSPF_MGR_IPC_IFMTUUNSET,
    OSPF_MGR_IPC_IFMTUIGNORESET,
    OSPF_MGR_IPC_IFMTUIGNOREUNSET,
    OSPF_MGR_IPC_IFAREAIDSET,
    OSPF_MGR_IPC_IFAREAIDUNSET,
    OSPF_MGR_IPC_IFDEADINTERVALSET,
    OSPF_MGR_IPC_IFDEADINTERVALUNSET,
    OSPF_MGR_IPC_IFHELLOINTERVALSET,
    OSPF_MGR_IPC_IFHELLOINTERVALUNSET,
    OSPF_MGR_IPC_IFRETRANSMITINTERVALSET,
    OSPF_MGR_IPC_IFRETRANSMITINTERVALUNSET,
    OSPF_MGR_IPC_IFTRANSMITDELAYSET,
    OSPF_MGR_IPC_IFTRANSMITDELAYUNSET,
    OSPF_MGR_IPC_IFPARAM_UNSET,
    OSPF_MGR_IPC_NETWORKSET,
    OSPF_MGR_IPC_NETWORKUNSET,
    OSPF_MGR_IPC_ROUTERIDSET,
    OSPF_MGR_IPC_ROUTERIDUNSET,
    OSPF_MGR_IPC_TIMERSET,
    OSPF_MGR_IPC_TIMERUNSET,
    OSPF_MGR_IPC_DEFAULTMETRICSET,
    OSPF_MGR_IPC_DEFAULTMETRICUNSET,
    OSPF_MGR_IPC_PASSIVEINTERFACESET,
    OSPF_MGR_IPC_PASSIVEINTERFACEUNSET,
    OSPF_MGR_IPC_COMPATIBLERFC1583SET,
    OSPF_MGR_IPC_COMPATIBLERFC1583UNSET,
    OSPF_MGR_IPC_AREASTUBSET,
    OSPF_MGR_IPC_AREASTUBUNSET,
    OSPF_MGR_IPC_AREASTUBNOSUMMARYSET,
    OSPF_MGR_IPC_AREASTUBNOSUMMARYUNSET,
    OSPF_MGR_IPC_AREADEFAULTCOSTSET,
    OSPF_MGR_IPC_AREADEFAULTCOSTUNSET,
    OSPF_MGR_IPC_AREARANGESET,
    OSPF_MGR_IPC_AREARANGENOADVERTISESET,
    OSPF_MGR_IPC_AREARANGEUNSET,
    OSPF_MGR_IPC_AREARANGESTATUSSET,
    OSPF_MGR_IPC_AREARANGEEFFECTSET,
    OSPF_MGR_IPC_AREANSSASET,
    OSPF_MGR_IPC_AREANSSAUNSET,
    OSPF_MGR_IPC_AREAVLINKSET,
    OSPF_MGR_IPC_AREAVLINKUNSET,
    OSPF_MGR_IPC_AREAAUTHENTICATIONTYPESET,
    OSPF_MGR_IPC_AREAAUTHENTICATIONTYPEUNSET,
    OSPF_MGR_IPC_GETNEXTPROCESSSTATUS,
    OSPF_MGR_IPC_GETNEXTAREAPARA,
    OSPF_MGR_IPC_GETAREAPARA,
    OSPF_MGR_IPC_GETAREARANGE,
    OSPF_MGR_IPC_GETNEXTAREARANGE,
    OSPF_MGR_IPC_GETNEXTROUTE,
    OSPF_MGR_IPC_GETOSPFMULTIPROCESSROUTENEXTHOPENTRY,
    OSPF_MGR_IPC_GETNEXTOSPFMULTIPROCESSROUTENEXTHOPENTRY,
    OSPF_MGR_IPC_GETNETWORKENTRY,
    OSPF_MGR_IPC_GETNEXTNETWORKENTRY,
    OSPF_MGR_IPC_AREASUMMARYSET,
    OSPF_MGR_IPC_GETAREAENTRY,
    OSPF_MGR_IPC_GETNEXTAREAENTRY,
    OSPF_MGR_IPC_GETSTUBAREAENTRY,
    OSPF_MGR_IPC_GETNEXTSTUBAREAENTRY,
    OSPF_MGR_IPC_GETNSSAAREAENTRY,
    OSPF_MGR_IPC_GETNEXTNSSAAREAENTRY,
    OSPF_MGR_IPC_STUBAREAMETRICSET,
    OSPF_MGR_IPC_STUBAREAMETRICTYPESET,
    OSPF_MGR_IPC_STUBAREASTATUSSET,
    OSPF_MGR_IPC_GETPASSIVEIF,
    OSPF_MGR_IPC_GETNEXTPASSIVEIF,
    OSPF_MGR_IPC_SUMMARY_ADDRSET,
    OSPF_MGR_IPC_SUMMAYR_ADDRUNSET,
    OSPF_MGR_IPC_GET_MULTIPROCSUMMARYADDRTABLE,
    OSPF_MGR_IPC_GETNEXT_MULTIPROCSUMMARYADDRTABLE,
    OSPF_MGR_IPC_GET_MULTIPROCIFTABLE,
    OSPF_MGR_IPC_GETNEXT_MULTIPROCIFTABLE,
    OSPF_MGR_IPC_AUTOCOSTSET,
    OSPF_MGR_IPC_AUTOCOSTUNSET,
    OSPF_MGR_IPC_GETAUTOCOST,
    OSPF_MGR_IPC_REDISTRIBUTE_PROTOSET,
    OSPF_MGR_IPC_REDISTRIBUTE_PROTOUNSET,
    OSPF_MGR_IPC_REDISTRIBUTE_METRICSET,
    OSPF_MGR_IPC_REDISTRIBUTE_METRICUNSET,
    OSPF_MGR_IPC_REDISTRIBUTE_METRICTYPESET,
    OSPF_MGR_IPC_REDISTRIBUTE_METRICTYPEUNSET,
    OSPF_MGR_IPC_REDISTRIBUTE_ROUTEMAPSET,
    OSPF_MGR_IPC_REDISTRIBUTE_ROUTEMAPUNSET,
    OSPF_MGR_IPC_REDISTRIBUTE_FILTERLISTNAMESET,
    OSPF_MGR_IPC_REDISTRIBUTE_FILTERLISTNAMEUNSET,
    OSPF_MGR_IPC_REDISTRIBUTE_TAGSET,
    OSPF_MGR_IPC_REDISTRIBUTE_TAGUNSET,
    OSPF_MGR_IPC_GET_MULTIPROCREDISTRIBUTETABLE,
    OSPF_MGR_IPC_GETNEXT_MULTIPROCREDISTRIBUTETABLE,
    OSPF_MGR_IPC_DEFAULTINFO_METRICSET,
    OSPF_MGR_IPC_DEFAULTINFO_METRICUNSET,
    OSPF_MGR_IPC_DEFAULTINFO_METRICTYPESET,
    OSPF_MGR_IPC_DEFAULTINFO_METRICTYPEUNSET,
    OSPF_MGR_IPC_DEFAULTINFO_ROUTEMAPSET,
    OSPF_MGR_IPC_DEFAULTINFO_ROUTEMAPUNSET,
    OSPF_MGR_IPC_DEFAULTINFO_ALWAYSSET,
    OSPF_MGR_IPC_DEFAULTINFO_ALWWAYSUNSET,
    OSPF_MGR_IPC_DEFAULTINFO_SET,
    OSPF_MGR_IPC_DEFAULTINFO_UNSET,
    OSPF_MGR_IPC_GETNEXT_NBR_ENTRY,
    OSPF_MGR_IPC_GETOSPFINTERFACEENTRY,
    OSPF_MGR_IPC_GETNEXT_OSPF_DATABASE_ROUTER,
    OSPF_MGR_IPC_CLEAR_OSPF_PROCESS,
    OSPF_MGR_IPC_CLEAR_OSPF_PROCESS_ALL,
    OSPF_MGR_IPC_GETNEXT_DATABASE_LSA_ENTRY,
    OSPF_MGR_IPC_GET_ABR_BORDER_ROUTER_ENTRY,
    OSPF_MGR_IPC_GET_ASBR_BORDER_ROUTER_ENTRY,
    OSPF_MGR_IPC_GETNEXT_OSPF_ENTRY,
    OSPF_MGR_IPC_GET_OSPF_ENTRY,
    OSPF_MGR_IPC_GET_MULTIPROCVIRTUALLINKENTRY,
    OSPF_MGR_IPC_GETNEXT_MULTIPROCVIRTUALLINKENTRY,
    OSPF_MGR_IPC_GETNEXT_VIRTUALLINKENTRY,
    OSPF_MGR_IPC_GET_MULTIPROCVIRTIFMD5KEY,
    OSPF_MGR_IPC_GETNEXT_MULTIPROCVIRTIFMD5KEY,
    OSPF_MGR_IPC_GET_MULTIPROCVIRTNBR,
    OSPF_MGR_IPC_GETNEXT_MULTIPROCVIRTNBR,
    /*wang.tong add*/
    OSPF_MGR_IPC_GET_IFPARAM_ENTRY,
    OSPF_MGR_IPC_GETNEXT_IFPARAM_ENTRY,
    OSPF_MGR_IPC_GETOPERATINGIFPARAMENTRY,
    OSPF_MGR_IPC_SET_IFPARAM_ENTRY,
    OSPF_MGR_IPC_SET_AREALIMIT,
    OSPF_MGR_IPC_GET_MULTI_PROCESS_System_ENTRY,
    OSPF_MGR_IPC_GETNEXT_MULTI_PROCESS_System_ENTRY,
    OSPF_MGR_IPC_GET_MULTI_PROCESS_NBR_ENTRY,
    OSPF_MGR_IPC_GETNEXT_MULTI_PROCESS_NBR_ENTRY,
    OSPF_MGR_IPC_GET_MULTI_PROCESS_LSDB_ENTRY,
    OSPF_MGR_IPC_GETNEXT_MULTI_PROCESS_LSDB_ENTRY,
    OSPF_MGR_IPC_GET_MULTI_PROCESS_EXT_LSDB_ENTRY,
    OSPF_MGR_IPC_GETNEXT_MULTI_PROCESS_EXT_LSDB_ENTRY,
    OSPF_MGR_IPC_GET_MULTI_PROCESS_IF_AUTH_MD5_ENTRY,
    OSPF_MGR_IPC_GETNEXT_MULTI_PROCESS_IF_AUTH_MD5_ENTRY,
    OSPF_MGR_IPC_SET_MULTI_PROCESS_IF_AUTH_MD5_ENTRY,
    OSPF_MGR_IPC_GETNEXT_OSPFINTERFACEENTRY,
    /*end wang.tong add */

    // peter add, for mib
    OSPF_MGR_IPC_GETROUTERID,
    OSPF_MGR_IPC_GETADMINSTAT,
    OSPF_MGR_IPC_SETADMINSTAT,
    OSPF_MGR_IPC_GETVERSIONNUMBER,
    OSPF_MGR_IPC_GETAREABDRRTRSTATUS,
    OSPF_MGR_IPC_GETASBDRRTRSTATUS,
    OSPF_MGR_IPC_SETASBDRRTRSTATUS,
    OSPF_MGR_IPC_GETEXTERNLSACOUNT,
    OSPF_MGR_IPC_GETEXTERNLSACKSUMSUM,
    OSPF_MGR_IPC_GETTOSSUPPORT,
    OSPF_MGR_IPC_SETTOSSUPPORT,
    OSPF_MGR_IPC_GETORIGINATENEWLSAS,
    OSPF_MGR_IPC_GETRXNEWLSAS,
    OSPF_MGR_IPC_GETEXTLSDBLIMIT,
    OSPF_MGR_IPC_SETEXTLSDBLIMIT,
    OSPF_MGR_IPC_GETMULTICASTEXTENSIONS,
    OSPF_MGR_IPC_SETMULTICASTEXTENSIONS,
    OSPF_MGR_IPC_GETEXITOVERFLOWINTERVAL,
    OSPF_MGR_IPC_SETEXITOVERFLOWINTERVAL,
    OSPF_MGR_IPC_GETDEMANDEXTENSIONS,
    OSPF_MGR_IPC_SETDEMANDEXTENSIONS,
    OSPF_MGR_IPC_SETAREASTATUS,
    OSPF_MGR_IPC_GETHOSTENTRY,
    OSPF_MGR_IPC_GETNEXTHOSTENTRY,
    OSPF_MGR_IPC_SETHOSTMETRIC,
    OSPF_MGR_IPC_SETHOSTSTATUS,
    OSPF_MGR_IPC_SETIFTYPE,
    OSPF_MGR_IPC_SETIFADMINSTAT,
    OSPF_MGR_IPC_SETIFAUTHKEY,
    OSPF_MGR_IPC_SETIFPOLLINTERVAL,
    OSPF_MGR_IPC_GETIFMETRICENTRY,
    OSPF_MGR_IPC_GETNEXTIFMETRICENTRY,
    OSPF_MGR_IPC_SETIFMETRICENTRY,

    OSPF_MGR_IPC_FOLLOWISASYNCHRONISMIPC,

    OSPF_MGR_IPC_INTERFACEUP,
    OSPF_MGR_IPC_INTERFACEDOWN,
};

/*****************************************
 **      ofpf_mgr ipc msg structure      **
 *****************************************
 */
typedef struct OSPF_MGR_IPCMsg_S
{
    union OSPF_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request command */
        BOOL_T result_bool;      /*respond bool return*/
        UI32_T result_ui32;      /*respond ui32 return*/
        SYS_TYPE_Get_Running_Cfg_T         result_running_cfg; /* For running config API */
    } type;

    union
    {
        OSPF_MGR_OSPF_ENTRY_T ospf_entry;
        OSPF_MGR_LSA_ENTRY_T ospf_lsa_entry;
        OSPF_MGR_NBR_ENTRY_T ospf_neighbor_entry;
        OSPF_TYPE_Area_Para_T    arg_area_para;
        OSPF_TYPE_Area_Range_T    arg_area_range;
        OSPF_TYPE_Route_T         arg_route;
        OSPF_TYPE_Vlink_T ospf_virtual_link_entry;
        OSPF_TYPE_MultiProcessVirtNbr_T ospf_virtual_nbr_entry;
        OSPF_MGR_ABR_BORDER_ROUTER_ENTRY_T ospf_abr_border_router_entry;
        OSPF_MGR_ASBR_BORDER_ROUTER_ENTRY_T ospf_asbr_border_router_entry;
        OSPF_TYPE_Network_Area_T arg_network_area;
        OSPF_TYPE_Area_T arg_area;
        OSPF_TYPE_Stub_Area_T arg_stub;
        OSPF_TYPE_Nssa_Area_T arg_nssa;
        OSPF_TYPE_Passive_If_T arg_pass_if;
        OspfMultiProcessRouteNexthopEntry_T ospf_route_nexthop_entry;
        OSPF_TYPE_HostEntry_T host_entry;
        OSPF_TYPE_IfMetricEntry_T if_metric_entry;
        UI32_T  ui32_v;
        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
        } arg_grp1;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
        } arg_grp2;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
        } arg_grp3;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
            UI32_T   arg5;
        } arg_grp4;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
            UI32_T   arg5;
            UI32_T   arg6;
        } arg_grp5;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            char     arg4[40];
        } arg_grp6;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI8_T    arg3;
            BOOL_T   arg4;
            struct pal_in4_addr arg5;
        } arg_grp7;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            BOOL_T   arg3;
            struct pal_in4_addr arg4;
        } arg_grp8;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            BOOL_T   arg4;
            struct pal_in4_addr arg5;
        } arg_grp9;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            char     arg3[OSPF_AUTH_SIMPLE_SIZE + 1];
            BOOL_T   arg4;
            struct pal_in4_addr arg5;
        } arg_grp10;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI8_T    arg3;
            char     arg4[OSPF_AUTH_MD5_SIZE + 1];
            BOOL_T   arg5;
            struct pal_in4_addr arg6;
        } arg_grp11;

        struct
        {
            UI32_T   arg1;
            OSPF_TYPE_OspfInterfac_T arg2;
        } arg_grp12;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
            OSPF_TYPE_Area_Nssa_Para_T   arg5;
        } arg_grp14;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
            OSPF_TYPE_Area_Virtual_Link_Para_T   arg5;
        } arg_grp15;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
            UI32_T   arg5;
            UI32_T   arg6;
            UI32_T   arg7;
        } arg_grp16;

        /*wang.tong add*/
        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            int arg3;
            OSPF_TYPE_IfParam_T arg4;
        }arg_grp_ifparam;

        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            UI32_T arg3;
            OSPF_TYPE_MultiProcessSystem_T arg4;
        }arg_grp_multiprocess;

        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            OSPF_TYPE_MultiProcessNbr_T arg3;
        }arg_grp_multiprocessnbr;

        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            OSPF_TYPE_MultiProcessLsdb_T arg3;
        }arg_grp_multiprocesslsdb;

        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            OSPF_TYPE_MultiProcessExtLsdb_T arg3;
        }arg_grp_multiprocessextlsdb;

        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            UI32_T arg3;
            OSPF_TYPE_MultiProcessIfAuthMd5_T arg4;
        }arg_grp_IfAuthMd5;
/*end wang.tong*/

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            OSPF_TYPE_Multi_Proc_Summary_Addr_T   arg3;
        } arg_grp_summary;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            OSPF_TYPE_Multi_Proc_Redist_T   arg3;
        } arg_grp_redist;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            OSPF_TYPE_Msg_OspfInterfac_T   arg3;
        } arg_grp_interface;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            char     arg3[20];
            char     arg4[20];
        } arg_grp_route_map;

        struct
        {
            UI32_T   arg1;
            UI32_T   arg2;
            char     arg3[20];
            char     arg4[20];
        } arg_grp_list_name;

        struct
        {
            UI32_T   var_ui32;
            char     var_char20[20];
        } ui32_char20;

    } data;
} OSPF_MGR_IPCMsg_T;


/* FUNCTION NAME:  OSPF_MGR_SetTransitionMode
* PURPOSE:
*    This function will set transition state flag.
*
* INPUT:
*    None.
*
* OUTPUT:
*    None.
*
* RETURN:
*    None.
*
* NOTES:
*    None.
*
*/
void OSPF_MGR_SetTransitionMode(void);

/* FUNCTION NAME:  OSPF_MGR_EnterTransitionMode
* PURPOSE:
*    This function will force OSPF to enter transition state.
*
* INPUT:
*    None.
*
* OUTPUT:
*    None.
*
* RETURN:
*    None.
*
* NOTES:
*    None.
*
*/
void OSPF_MGR_EnterTransitionMode(void);

/* FUNCTION NAME:  OSPF_MGR_EnterMasterMode
* PURPOSE:
*    This function will force OSPF to enter master state.
*
* INPUT:
*    None.
*
* OUTPUT:
*    None.
*
* RETURN:
*    None.
*
* NOTES:
*    None.
*
*/
void OSPF_MGR_EnterMasterMode(void);

/* FUNCTION NAME:  OSPF_MGR_EnterSlaveMode
* PURPOSE:
*    This function will force OSPF to enter slave state.
*
* INPUT:
*    None.
*
* OUTPUT:
*    None.
*
* RETURN:
*    None.
*
* NOTES:
*    None.
*
*/
void OSPF_MGR_EnterSlaveMode(void);

/* FUNCTION NAME : OSPF_MGR_HandleIPCReqMsg
* PURPOSE:
*      Handle the ipc request received from mgr queue.
*
* INPUT:
*      sysfun_msg_p  --  The ipc request for OSPF_MGR.
*
* OUTPUT:
*      sysfun_msg_p  --  The ipc response to send when return value is TRUE
*
* RETURN:
*      TRUE   --  A response is required to send
*      FALSE  --  Need not to send response.
*
* NOTES:
*      1. The buffer length in sysfun_msg_p must be large enough for sending
*         all possible response messages.
*/
BOOL_T OSPF_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *sysfun_msg_p);

/* FUNCTION NAME : OSPF_MGR_RouterOspfSet
* PURPOSE:
*      Enable OSPF for a instance.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_RouterOspfSet(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_MGR_RouterOspfUnset
* PURPOSE:
*      Disable OSPF for a instance.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_RouterOspfUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_MGR_InterfaceAdd
* PURPOSE:
*      Add ospf interface.
*
* INPUT:
*      vr_id,
*      vrf_id
*      ifindex
*      mtu
*      bandwidth
*      if_flags
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_InterfaceAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T mtu, UI32_T bandwidth, UI32_T if_flags);

/* FUNCTION NAME : OSPF_MGR_InterfaceDelete
* PURPOSE:
*      Delete ospf interface.
*
* INPUT:
*      vr_id,
*      vrf_id
*      ifindex
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_InterfaceDelete(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : OSPF_MGR_IpAddressAdd
* PURPOSE:
*      Add IP address.
*
* INPUT:
*      vr_id,
*      vrf_id
*      ifindex
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IpAddressAdd(UI32_T vr_id, UI32_T ifindex,UI32_T ip_addr, UI32_T ip_mask, UI32_T primary);

/* FUNCTION NAME : OSPF_MGR_IpAddressDelete
* PURPOSE:
*      Delete IP address.
*
* INPUT:
*      vr_id,
*      vrf_id
*      ifindex
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IpAddressDelete(UI32_T vr_id, UI32_T ifindex,UI32_T ip_addr, UI32_T ip_mask);

/* FUNCTION NAME : OSPF_MGR_InterfaceUp
* PURPOSE:
*      Interface up
*
* INPUT:
*      vr_id,
*      vrf_id
*      ifindex
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_InterfaceUp(UI32_T vr_id, UI32_T ifindex);

/* FUNCTION NAME : OSPF_MGR_InterfaceDown
* PURPOSE:
*      Interface down.
*
* INPUT:
*      vr_id,
*      vrf_id
*      ifindex
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_InterfaceDown(UI32_T vr_id, UI32_T ifindex);

/* FUNCTION NAME :      OSPF_MGR_IfAuthenticationTypeSet
* PURPOSE:
*      Set OSPF interface authentication type.
*
* INPUT:
*      vr_id,
*      ifindex
*      type
*      addr_flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfAuthenticationTypeSet(UI32_T vr_id, UI32_T ifindex, UI8_T type, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfAuthenticationTypeUnset
* PURPOSE:
*      Unset OSPF interface authentication type.
*
* INPUT:
*      vr_id,
*      ifindex
*      addr_flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfAuthenticationTypeUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfAuthenticationKeySet
* PURPOSE:
*      Set OSPF interface authentication key.
*
* INPUT:
*      vr_id,
*      ifindex
*      auth_key
*      addr_ flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfAuthenticationKeySet(UI32_T vr_id, UI32_T ifindex, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfAuthenticationKeyUnset
* PURPOSE:
*      Unset OSPF interface authentication key.
*
* INPUT:
*      vr_id,
*      ifindex
*      addr_flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfAuthenticationKeyUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfMessageDigestKeySet
* PURPOSE:
*      Set OSPF interface message digest key.
*
* INPUT:
*      vr_id,
*      ifindex
*      key_id
*      auth_key
*      addr_ flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfMessageDigestKeySet(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfMessageDigestKeyUnset
* PURPOSE:
*      Unset OSPF interface message digest key.
*
* INPUT:
*      vr_id,
*      ifindex
*      key_id
*      addr_flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfMessageDigestKeyUnset(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfPrioritySet
* PURPOSE:
*      Set OSPF interface priority.
*
* INPUT:
*      vr_id,
*      ifindex
*      priority
*      addr_ flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfPrioritySet(UI32_T vr_id, UI32_T ifindex, UI8_T priority, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfPriorityUnset
* PURPOSE:
*      Unset OSPF interface priority.
*
* INPUT:
*      vr_id,
*      ifindex
*      addr_flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfPriorityUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfCostSet
* PURPOSE:
*      Set OSPF interface cost.
*
* INPUT:
*      vr_id,
*      ifindex
*      cost
*      addr_ flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfCostSet(UI32_T vr_id, UI32_T ifindex, UI32_T cost, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfCostUnset
* PURPOSE:
*      Unset OSPF interface cost.
*
* INPUT:
*      vr_id,
*      ifindex
*      addr_flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfCostUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);
/* FUNCTION NAME :      OSPF_MGR_IfMtuSet
* PURPOSE:
*      Set OSPF interface mtu.
*
* INPUT:
*      vr_id,
*      ifindex
*      mtu
*      addr_ flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfMtuSet(UI32_T vr_id, UI32_T ifindex, UI32_T mtu, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfMtuUnset
* PURPOSE:
*      Unset OSPF interface mtu.
*
* INPUT:
*      vr_id,
*      ifindex
*      addr_flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfMtuUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfMtuIgnoreSet
* PURPOSE:
*      Set OSPF interface mtu ignore.
*
* INPUT:
*      vr_id,
*      ifindex
*      mtu_ignore
*      addr_ flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfMtuIgnoreSet(UI32_T vr_id, UI32_T ifindex, UI32_T mtu_ignore, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfMtuIgnoreUnset
* PURPOSE:
*      Unset OSPF interface mtu ignore.
*
* INPUT:
*      vr_id,
*      ifindex
*      addr_flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfMtuIgnoreUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfDeadIntervalSet
* PURPOSE:
*      Set OSPF interface dead interval.
*
* INPUT:
*      vr_id,
*      ifindex
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfDeadIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfDeadIntervalUnset
* PURPOSE:
*      Unset OSPF interface dead interval.
*
* INPUT:
*      vr_id,
*      ifindex
*      addr_flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfDeadIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfHelloIntervalSet
* PURPOSE:
*      Set OSPF interface hello interval.
*
* INPUT:
*      vr_id,
*      ifindex
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfHelloIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfHelloIntervalUnset
* PURPOSE:
*      Unset OSPF interface hello interval.
*
* INPUT:
*      vr_id,
*      ifindex
*      addr_flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfHelloIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfRetransmitIntervalSet
* PURPOSE:
*      Set OSPF interface retransmit interval.
*
* INPUT:
*      vr_id,
*      ifindex
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfRetransmitIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfRetransmitIntervalUnset
* PURPOSE:
*      Unset OSPF interface retransmit interval.
*
* INPUT:
*      vr_id,
*      ifindex
*      addr_flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfRetransmitIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfTransmitDelaySet
* PURPOSE:
*      Set OSPF interface transmit delay.
*
* INPUT:
*      vr_id,
*      ifindex
*      delay
*      addr_ flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfTransmitDelaySet(UI32_T vr_id, UI32_T ifindex, UI32_T delay, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :      OSPF_MGR_IfTransmitDelayUnset
* PURPOSE:
*      Unset OSPF interface transmit delay.
*
* INPUT:
*      vr_id,
*      ifindex
*      addr_flag
*      addr
*
* OUTPUT:
*      None.
*
* RETURN:
*      Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_IfTransmitDelayUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);


UI32_T OSPF_MGR_IfParamUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);


/* FUNCTION NAME : OSPF_MGR_NetworkSet
* PURPOSE:
*     Set ospf network.
*
* INPUT:
*      vr_id,
*      proc_id,
*      network_addr,
*      masklen,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_NetworkSet(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_MGR_NetworkUnset
* PURPOSE:
*     Delete ospf network.
*
* INPUT:
*      vr_id,
*      proc_id,
*      network_addr,
*      masklen,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_NetworkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_MGR_RouterIdSet
* PURPOSE:
*     Set ospf router id.
*
* INPUT:
*      vr_id,
*      proc_id,
*      router_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_RouterIdSet(UI32_T vr_id, UI32_T proc_id, UI32_T router_id);

/* FUNCTION NAME : OSPF_MGR_RouterIdUnset
* PURPOSE:
*     Unset ospf router id.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_RouterIdUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_MGR_TimerSet
* PURPOSE:
*     Set ospf timer value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      delay,
*      hold.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_TimerSet(UI32_T vr_id, UI32_T proc_id, UI32_T delay, UI32_T hold);

/* FUNCTION NAME : OSPF_MGR_TimerUnset
* PURPOSE:
*     Set ospf timer value to default value.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_TimerUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_MGR_DefaultMetricSet
* PURPOSE:
*     Set ospf default metric value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      metric.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_DefaultMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric);

/* FUNCTION NAME : OSPF_MGR_DefaultMetricUnset
* PURPOSE:
*     Unset ospf default metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_DefaultMetricUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_MGR_PassiveIfSet
* PURPOSE:
*     Set ospf passive interface.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id,
*      entry->ifindex,
*      entry->addr.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_PassiveIfSet(OSPF_TYPE_Passive_If_T *entry);

/* FUNCTION NAME : OSPF_MGR_PassiveIfUnset
* PURPOSE:
*     Unset ospf passive interface.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id,
*      entry->ifindex,
*      entry->addr.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_PassiveIfUnset(OSPF_TYPE_Passive_If_T *entry);

/* FUNCTION NAME : OSPF_MGR_CompatibleRfc1853Set
* PURPOSE:
*     Set ospf compatible rfc1853.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_CompatibleRfc1853Set(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_MGR_CompatibleRfc1853Unset
* PURPOSE:
*     Unset ospf compatible rfc1853.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_CompatibleRfc1853Unset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_MGR_GetOspfInterfaceEntry
* PURPOSE:
*     Get  ospf interface entry.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       Success/Error
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetOspfInterfaceEntry(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetOspfInterfaceEntry
* PURPOSE:
*     Get  ospf interface entry for MIB.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       Success/Error
*
* NOTES:
*      None.
*/
int OSPF_MGR_GetMultiProcIfEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_Msg_OspfInterfac_T *data);

/* FUNCTION NAME : OSPF_MGR_GetNextMultiProcIfEntry
* PURPOSE:
*     Getnext  ospf interface entry for MIB.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       Success/Error
*OSPF_MGR_GetNextMultiProcIfEntry
* NOTES:
*      None.
*/
int OSPF_MGR_GetNextMultiProcIfEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_Msg_OspfInterfac_T *data);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - OSPF_MGR_BackDoor_Main
 * ------------------------------------------------------------------------
 * FUNCTION : This function ititiates the backdoor function
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void OSPF_MGR_BackDoor_Main(void *handle, UI32_T member_id);

/* FUNCTION NAME : OSPF_MGR_AreaStubSet
* PURPOSE:
*     Set ospf area stub.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaStubSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_MGR_AreaStubUnset
* PURPOSE:
*     Unset ospf area stub.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaStubUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);
/*============================================================
 *FUNCTION NAME: OSPF_MGR_Get_MultiProcSummaryAddrEntry
 *PURPOSE:
 *
 *INPUT:
 *
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */
UI32_T OSPF_MGR_Get_MultiProcSummaryAddrEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Multi_Proc_Summary_Addr_T *data);
/*============================================================
 *FUNCTION NAME: OSPF_MGR_GetNext_MultiProcSummaryAddrEntry
 *PURPOSE:
 *
 *INPUT:
 *
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */
UI32_T OSPF_MGR_GetNext_MultiProcSummaryAddrEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Multi_Proc_Summary_Addr_T *data);


/* FUNCTION NAME : OSPF_MGR_AreaStubNoSummarySet
* PURPOSE:
*     Set ospf area stub no summary.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaStubNoSummarySet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_MGR_AreaStubNoSummaryUnset
* PURPOSE:
*     Unset ospf area stub no summary.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaStubNoSummaryUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_MGR_AreaDefaultCostSet
* PURPOSE:
*     Set ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      cost.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaDefaultCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T cost);

/* FUNCTION NAME : OSPF_MGR_AreaDefaultCostUnset
* PURPOSE:
*     Unset ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaDefaultCostUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_MGR_AreaRangeSet
* PURPOSE:
*     Set ospf area range.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaRangeSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen);

/* FUNCTION NAME : OSPF_MGR_AreaRangeNoAdvertiseSet
* PURPOSE:
*     Set ospf area range no advertise.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaRangeNoAdvertiseSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen);

/* FUNCTION NAME : OSPF_MGR_AreaRangeUnset
* PURPOSE:
*     Unset ospf area range.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaRangeUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen);

/* FUNCTION NAME : OSPF_MGR_AreaNssaSet
* PURPOSE:
*     Set ospf area nssa.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      nssa_para.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaNssaSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, OSPF_TYPE_Area_Nssa_Para_T *nssa_para);

/* FUNCTION NAME : OSPF_MGR_AreaNssaUnset
* PURPOSE:
*     Unset ospf nssa area parameter or the whole area (if flag is 0)
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      flag.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaNssaUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T flag);

/* FUNCTION NAME : OSPF_MGR_GetNextProcessStatus
* PURPOSE:
*     Unset ospf area nssa.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      proc_id.
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetNextProcessStatus(UI32_T vr_id, UI32_T *proc_id);

/* FUNCTION NAME : OSPF_MGR_AreaVirtualLinkSet
* PURPOSE:
*     Set ospf area virtual link.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      vlink_para.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaVirtualLinkSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, OSPF_TYPE_Area_Virtual_Link_Para_T *vlink_para);

/* FUNCTION NAME : OSPF_MGR_AreaVirtualLinkUnset
* PURPOSE:
*     Unset ospf area virtual link.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      vlink_para.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaVirtualLinkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, OSPF_TYPE_Area_Virtual_Link_Para_T *vlink_para);

/* FUNCTION NAME : OSPF_MGR_GetNextAreaPara
* PURPOSE:
*     Get nextospf area parameters.
*
* INPUT:
*     entry->first_flag,
*     entry->vr_id,
*     entry->proc_id
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetNextAreaPara(OSPF_TYPE_Area_Para_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetAreaPara
* PURPOSE:
*     Get  area parameters.
*
* INPUT:
*
*     entry->vr_id,
*     entry->proc_id
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetAreaPara(OSPF_TYPE_Area_Para_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetOspfEntry
* PURPOSE:
*     Get  ospf entry.
*
* INPUT:
*
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetOspfEntry(OSPF_MGR_OSPF_ENTRY_T *ospf_entry_p);

/* FUNCTION NAME : OSPF_MGR_GetAreaRangeTable
* PURPOSE:
*     Get ospf area range.
*
* INPUT:
*     entry->vr_id
*     entry->proc_id
*     entry->area_id
*     entry->type
*     entry->range_addr,
*     entry->range_mask;
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetAreaRangeTable(OSPF_TYPE_Area_Range_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetNextAreaRangeTable
* PURPOSE:
*     Get nextospf area range.
*
* INPUT:
*     entry->indexlen
*     entry->vr_id
*     entry->proc_id
*     entry->area_id
*     entry->type
*     entry->range_addr,
*     entry->range_mask
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      1.if entry->indexlen is 0,get the first entry in the proc_id
*      2.key are vr_id, proc_id, area_id, type, range_addr and range_mask.
*/
UI32_T OSPF_MGR_GetNextAreaRangeTable(OSPF_TYPE_Area_Range_T *entry);

/*============================================================
 *FUNCTION NAME: OSPF_MGR_Get_MultiProcRedistEntry
 *PURPOSE:
 *
 *INPUT:
 *
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */
UI32_T OSPF_MGR_Get_MultiProcRedistEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_Multi_Proc_Redist_T *data);
/*============================================================
 *FUNCTION NAME: OSPF_MGR_GetNext_MultiProcRedistEntry
 *PURPOSE:
 *
 *INPUT:
 *
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */
UI32_T OSPF_MGR_GetNext_MultiProcRedistEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_Multi_Proc_Redist_T *data);



/* FUNCTION NAME : OSPF_MGR_GetNextRoute
* PURPOSE:
*     Get nextospf route information.
*
* INPUT:
*     entry
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetNextRoute(OSPF_TYPE_Route_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetMultiProcVirtualLinkEntry
* PURPOSE:
*     Get  virtual link entry of mib.
*
* INPUT:
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetMultiProcVirtualLinkEntry(OSPF_TYPE_Vlink_T *entry);


/* FUNCTION NAME : OSPF_MGR_GetNextMultiProcVirtualLinkEntry
* PURPOSE:
*     Get next virtual link entry of mib.
*
* INPUT:
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetNextMultiProcVirtualLinkEntry(OSPF_TYPE_Vlink_T *entry);


/* FUNCTION NAME : OSPF_MGR_GetNextVirtualLinkEntry
* PURPOSE:
*     Get next virtual link entry.
*
* INPUT:
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetNextVirtualLinkEntry(OSPF_TYPE_Vlink_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetMultiProcVirtIfMd5Entry
* PURPOSE:
*     Get  virtual link md5 key  of mib.
*
* INPUT:
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetMultiProcVirtIfMd5Entry(OSPF_TYPE_Vlink_T *entry);


/* FUNCTION NAME : OSPF_MGR_GetNextMultiProcVirtIfMd5Entry
* PURPOSE:
*     Get next virtual link md5 key of mib.
*
* INPUT:
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetNextMultiProcVirtIfMd5Entry(OSPF_TYPE_Vlink_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetMultiProcVirtNbrEntry
* PURPOSE:
*     Get  virtual link nbr of mib.
*
* INPUT:
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetMultiProcVirtNbrEntry(OSPF_TYPE_MultiProcessVirtNbr_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetNextMultiProcVirtNbrEntry
* PURPOSE:
*     Get next virtual link nbr of mib.
*
* INPUT:
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetNextMultiProcVirtNbrEntry(OSPF_TYPE_MultiProcessVirtNbr_T *entry);


/* FUNCTION NAME : OSPF_MGR_GetNetworkAreaTable
* PURPOSE:
*     Get network entry.
*
* INPUT:
*      entry->vr_id;
*      entry->proc_id;
*      entry->network_addr;
*      entry->network_pfx
*
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetNetworkAreaTable(OSPF_TYPE_Network_Area_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetNextNetworkAreaTable
* PURPOSE:
*     Get next network entry.
*
* INPUT:
*      entry->indexlen;
*      entry->vr_id;
*      entry->proc_id;
*      entry->network_addr;
*      entry->network_pfx
*
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      1.if entry->indexlen is 0,get the first entry in the proc_id
*      2.key are vr_id, proc_id, network_addr and network_pfx.
*/
UI32_T OSPF_MGR_GetNextNetworkAreaTable(OSPF_TYPE_Network_Area_T *entry);

/* FUNCTION NAME : OSPF_MGR_AreaSummarySet
* PURPOSE:
*     Get next network entry.
*
* INPUT:
*      vr_id;
*      proc_id;
*      area_id;
*      val
*
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaSummarySet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T val);

/* FUNCTION NAME : OSPF_MGR_GetAreaTable
* PURPOSE:
*     Get area entry.
*
* INPUT:
*      entry->vr_id;
*      entry->proc_id;
*      entry->area_id;
*
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetAreaTable(OSPF_TYPE_Area_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetNextAreaTable
* PURPOSE:
*     Get next area entry.
*
* INPUT:
*      entry->indexlen;
*      entry->vr_id;
*      entry->proc_id;
*      entry->area_id;
*
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      1.if entry->indexlen is 0,get the first entry in the proc_id
*      2.key are vr_id, proc_id, area_id.
*/
UI32_T OSPF_MGR_GetNextAreaTable(OSPF_TYPE_Area_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetStubAreaTable
* PURPOSE:
*     Get stub area entry.
*
* INPUT:
*      entry->vr_id;
*      entry->proc_id;
*      entry->area_id;
*      entry->stub_tos;
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetStubAreaTable(OSPF_TYPE_Stub_Area_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetNextStubAreaTable
* PURPOSE:
*     Get next stub area entry.
*
* INPUT:
*      entry->indexlen;
*      entry->vr_id;
*      entry->proc_id;
*      entry->area_id;
*      entry->stub_tos;
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      1.if entry->indexlen is 0,get the first entry in the proc_id
*      2.key are vr_id, proc_id, area_id and stub_tos.
*/
UI32_T OSPF_MGR_GetNextStubAreaTable(OSPF_TYPE_Stub_Area_T *entry);


/* FUNCTION NAME : OSPF_MGR_StubAreaMetricSet
* PURPOSE:
*     Set stub area metric(default-cost).
*
* INPUT:
*      vr_id;
*      proc_id;
*      area_id;
*      stub_tos;
*      metric;
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_StubAreaMetricSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id, UI32_T tos, UI32_T metric);

/* FUNCTION NAME : OSPF_MGR_StubAreaStatusSet
* PURPOSE:
*     Set stub area rowstatus.
*
* INPUT:
*      vr_id;
*      proc_id;
*      area_id;
*      stub_tos;
*      status;
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_StubAreaStatusSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id, UI32_T tos, UI32_T status);

/* FUNCTION NAME : OSPF_MGR_AreaAggregateStatusSet
* PURPOSE:
*     set ospf area range status.
*
* INPUT:
*    vr_id
*    proc_id
*    area_id
*    type
*    range_addr,
*    range_mask,
*    status;
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaAggregateStatusSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id,UI32_T type,UI32_T range_addr,UI32_T range_mask,UI32_T status);

/* FUNCTION NAME : OSPF_MGR_AreaAggregateEffectSet
* PURPOSE:
*     set ospf area range effect.
*
* INPUT:
*    vr_id
*    proc_id
*    area_id
*    type
*    range_addr,
*    range_mask,
*    effect;
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_AreaAggregateEffectSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id,UI32_T type,UI32_T range_addr,UI32_T range_mask,UI32_T effect);

/* FUNCTION NAME : OSPF_MGR_GetNssaTable
* PURPOSE:
*     Get nssa area entry.
*
* INPUT:
*      entry->vr_id;
*      entry->proc_id;
*      entry->area_id;
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetNssaTable(OSPF_TYPE_Nssa_Area_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetNextNssaTable
* PURPOSE:
*     Get next nssa area entry.
*
* INPUT:
*      entry->indexlen;
*      entry->vr_id;
*      entry->proc_id;
*      entry->area_id;
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      1.if entry->indexlen is 0,get the first entry in the proc_id
*      2.key are vr_id, proc_id and area_id.
*/
UI32_T OSPF_MGR_GetNextNssaTable(OSPF_TYPE_Nssa_Area_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetPassIfTable
* PURPOSE:
*     Get ospf passive interface.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id,
*      entry->ifindex,
*      entry->addr.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      key are vr_id, proc_id, ifindex and addr.
*/
UI32_T OSPF_MGR_GetPassIfTable(OSPF_TYPE_Passive_If_T *entry);

/* FUNCTION NAME : OSPF_MGR_GetNextPassIfTable
* PURPOSE:
*     Get next ospf passive interface.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id,
*      entry->ifindex,
*      entry->addr.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      key are vr_id, proc_id, ifindex and addr.
*/
UI32_T OSPF_MGR_GetNextPassIfTable(OSPF_TYPE_Passive_If_T *entry);

/*wang.tong add*/

/* FUNCTION NAME : OSPF_MGR_GetIfParamEntry
* PURPOSE:
*     Get IfParam entry.
*
* INPUT:
*      vr_id
*      vrf_id
*      entry
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetIfParamEntry(UI32_T vr_id, UI32_T vrf_id , OSPF_TYPE_IfParam_T *entry);



/* FUNCTION NAME : OSPF_MGR_GetNextIfParamEntry
* PURPOSE:
*     Get next IfParam entry.
*
* INPUT:
*      vr_id
*      vrf_id
*      indexlen
*      entry
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetNextIfParamEntry(UI32_T vr_id, UI32_T vrf_id, int indexlen, OSPF_TYPE_IfParam_T *entry);

/* FUNCTION NAME : OSPF_MGR_SetIfParamEntry
* PURPOSE:
*     Set IfParam entry.
*
* INPUT:
*      vr_id
*      vrf_id
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_SetIfParamEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_IfParam_T *entry);

/* FUNCTION NAME: OSPF_MGR_GetOperatingIfParamEntry
* PURPOSE:
*     Get operating IfParam entry for address or default value.
*
* INPUT:
*      vr_id
*      vrf_id
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetOperatingIfParamEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_IfParam_T *entry);

/*============================================================
 *FUNCTION NAME: OSPF_MGR_SetAreaLimit
 *PURPOSE: Set area limit.
 *
 *INPUT:   vr_id, proc_id, status
 *
 *OUTPUT:
 *
 *NOTES:
 *
 *=============================================================
 */

UI32_T OSPF_MGR_AreaLimitSet(UI32_T vr_id, UI32_T proc_id, UI32_T limit);


/*============================================================
 *FUNCTION NAME: OSPF_MGR_GetMultiProcessSystemEntry
 *PURPOSE:
 *
 *INPUT:
 *              vr_id
 *              proc_id
 *              entry
 *
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */
UI32_T OSPF_MGR_GetMultiProcessSystemEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_MultiProcessSystem_T *entry);


/*============================================================
 *FUNCTION NAME: OSPF_MGR_GetNextMultiProcessSystemEntry
 *PURPOSE:
 *
 *INPUT:
 *              vr_id
 *              indexlen
 *              entry
 *
 *OUTPUT:entry
 *
 *RETRUN:
 *
 *NOTES:
 *       1.if entry->indexlen is 0,get the first entry in the proc_id
 *=============================================================
 */
UI32_T OSPF_MGR_GetNextMultiProcessSystemEntry(UI32_T vr_id, UI32_T vrf_id, UI32_T indexlen, OSPF_TYPE_MultiProcessSystem_T *entry);

/*============================================================
 *FUNCTION NAME: OSPF_MGR_GetMultiProcessNbrEntry
 *PURPOSE:
 *
 *INPUT:
 *              vr_id
 *              entry
 *
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */
UI32_T OSPF_MGR_GetMultiProcessNbrEntry(UI32_T vr_id, OSPF_TYPE_MultiProcessNbr_T *entry);



/*============================================================
 *FUNCTION NAME: OSPF_MGR_GetNextMultiProcessNbrEntry
 *PURPOSE:
 *
 *INPUT:
 *              vr_id
 *              indexlen
 *              entry
 *
 *OUTPUT:entry
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */
UI32_T OSPF_MGR_GetNextMultiProcessNbrEntry(UI32_T vr_id, UI32_T indexlen,  OSPF_TYPE_MultiProcessNbr_T *entry);


/*============================================================
 *FUNCTION NAME: OSPF_MGR_GetMultiProcessLsdbEntry
 *PURPOSE:
 *
 *INPUT:
 *              vr_id
 *              entry
 *
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */
UI32_T OSPF_MGR_GetMultiProcessLsdbEntry(UI32_T vr_id, OSPF_TYPE_MultiProcessLsdb_T *entry);



/*============================================================
 *FUNCTION NAME: OSPF_MGR_GetNextMultiProcessLsdbEntry
 *PURPOSE:
 *
 *INPUT:
 *              vr_id
 *              indexlen
 *              entry
 *
 *OUTPUT:entry
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */
UI32_T OSPF_MGR_GetNextMultiProcessLsdbEntry(UI32_T vr_id, UI32_T indexlen,  OSPF_TYPE_MultiProcessLsdb_T *entry);


/*============================================================
 *FUNCTION NAME: OSPF_MGR_GetMultiProcessExtLsdbEntry
 *PURPOSE:
 *
 *INPUT:
 *              vr_id
 *              entry
 *
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */
UI32_T OSPF_MGR_GetMultiProcessExtLsdbEntry(UI32_T vr_id, OSPF_TYPE_MultiProcessExtLsdb_T *entry);



/*============================================================
 *FUNCTION NAME: OSPF_MGR_GetNextMultiProcessExtLsdbEntry
 *PURPOSE:
 *
 *INPUT:
 *              vr_id
 *              indexlen
 *              entry
 *
 *OUTPUT:entry
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */
UI32_T OSPF_MGR_GetNextMultiProcessExtLsdbEntry(UI32_T vr_id, UI32_T indexlen,  OSPF_TYPE_MultiProcessExtLsdb_T *entry);

/*============================================================
 *FUNCTION NAME: OSPF_MGR_GetIfAuthMd5Key
 *PURPOSE:
 *
 *INPUT:
 *              vr_id
 *              vrf_id
 *              entry
 *
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */

UI32_T OSPF_MGR_GetIfAuthMd5Key(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_MultiProcessIfAuthMd5_T *entry);

/*============================================================
 *FUNCTION NAME: OSPF_MGR_GetNextIfAuthMd5Key
 *PURPOSE:
 *
 *INPUT:
 *              vr_id
 *              vrf_id
 *              indexlen
 *              entry
 *
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */
UI32_T OSPF_MGR_GetNextIfAuthMd5Key(UI32_T vr_id, UI32_T vrf_id, UI32_T indexlen, OSPF_TYPE_MultiProcessIfAuthMd5_T *entry);

/*============================================================
 *FUNCTION NAME: OSPF_MGR_SetIfAuthMd5Key
 *PURPOSE:
 *
 *INPUT:
 *              vr_id
 *              vrf_id
 *              entry
 *
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES:
 *
 *=============================================================
 */

UI32_T OSPF_MGR_SetIfAuthMd5Key(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_MultiProcessIfAuthMd5_T *entry);


/* FUNCTION NAME : OSPF_MGR_GetNextOspfIfEntryByIfindex
* PURPOSE:
*     Get nex ospf interface entry by ifindex.
*
* INPUT:
*      vr_id,
*      entry,
*
* OUTPUT:
*      None
*
* RETURN:
*
* NOTES:
*      None.
*/
UI32_T OSPF_MGR_GetNextOspfIfEntryByIfindex(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry);


/*end .wang.tong*/

/* FUNCTION NAME : OSPF_MGR_GetOspfMultiProcessRouteNexthopEntry
 * PURPOSE:
 *      Get OSPF route information for SNMP.
 *
 * INPUT:
 *      entry_p
 *      key are,
 *      process_id  -- Process ID of an OSPF instance.
 *      dest        -- The destination IP address of this route.
 *      pfx_len     -- The prefix length of this route.
 *      next_hop    -- The nexthop IP address of this route.
 *
 * OUTPUT:
 *      entry_p
 *
 * RETURN:
 *      OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T OSPF_MGR_GetOspfMultiProcessRouteNexthopEntry(OspfMultiProcessRouteNexthopEntry_T *entry_p);

/* FUNCTION NAME : OSPF_MGR_GetNextOspfMultiProcessRouteNexthopEntry
 * PURPOSE:
 *      Get next OSPF route information for SNMP.
 *
 * INPUT:
 *      entry_p
 *      key are,
 *      process_id  -- Process ID of an OSPF instance.
 *      dest        -- The destination IP address of this route.
 *      pfx_len     -- The prefix length of this route.
 *      next_hop    -- The nexthop IP address of this route.
 *
 * OUTPUT:
 *      entry_p
 *
 * RETURN:
 *      OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T OSPF_MGR_GetNextOspfMultiProcessRouteNexthopEntry(OspfMultiProcessRouteNexthopEntry_T *entry_p);

// peter add, for mib
UI32_T OSPF_MGR_GetRouterId(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_GetAdminStat(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_SetAdminStat(UI32_T vr_id, UI32_T proc_id, UI32_T status);
UI32_T OSPF_MGR_GetVersionNumber(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_GetAreaBdrRtrStatus(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_GetASBdrRtrStatus (UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_SetASBdrRtrStatus(UI32_T vr_id, UI32_T proc_id, UI32_T status);
UI32_T OSPF_MGR_GetExternLsaCount(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_GetExternLsaCksumSum(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_GetTOSSupport(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_SetTOSSupport(UI32_T vr_id, UI32_T proc_id, UI32_T status);
UI32_T OSPF_MGR_GetOriginateNewLsas(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_GetRxNewLsas(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_GetExtLsdbLimit(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_SetExtLsdbLimit(UI32_T vr_id, UI32_T proc_id, UI32_T limit);
UI32_T OSPF_MGR_GetMulticastExtensions(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_SetMulticastExtensions(UI32_T vr_id, UI32_T proc_id, UI32_T status);
UI32_T OSPF_MGR_GetExitOverflowInterval(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_SetExitOverflowInterval(UI32_T vr_id, UI32_T proc_id, UI32_T interval);
UI32_T OSPF_MGR_GetDemandExtensions(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_MGR_SetDemandExtensions(UI32_T vr_id, UI32_T proc_id, UI32_T status);

UI32_T OSPF_MGR_StubAreaMetricTypeSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id, UI32_T tos, UI32_T metric_type);

UI32_T OSPF_MGR_SetAreaStatus(OSPF_TYPE_Area_T *area);

UI32_T OSPF_MGR_SetIfType(UI32_T ip_address, UI32_T if_type);
UI32_T OSPF_MGR_SetIfAdminStat(UI32_T ip_address, UI32_T status);

UI32_T OSPF_MGR_SetIfPollInterval(UI32_T ip_address, UI32_T value);

UI32_T OSPF_MGR_GetHostEntry(OSPF_TYPE_HostEntry_T *host_entry_p);
UI32_T OSPF_MGR_GetNextHostEntry(OSPF_TYPE_HostEntry_T *host_entry_p);

UI32_T OSPF_MGR_SetHostMetric(UI32_T vr_id, UI32_T proc_id,
              UI32_T ip_address, UI32_T tos, UI32_T metric);

UI32_T OSPF_MGR_SetHostStatus(UI32_T vr_id, UI32_T proc_id,
              UI32_T ip_address, UI32_T tos, UI32_T metric);


UI32_T OSPF_MGR_GetIfMetricEntry(OSPF_TYPE_IfMetricEntry_T *entry_p);
UI32_T OSPF_MGR_GetNextIfMetricEntry(OSPF_TYPE_IfMetricEntry_T *entry_p);
UI32_T OSPF_MGR_SetIfMetricEntry(OSPF_TYPE_IfMetricEntry_T *entry_p);

UI32_T OSPF_MGR_AreaAuthenticationTypeSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T type);
UI32_T OSPF_MGR_AreaAuthenticationTypeUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id);
#endif


