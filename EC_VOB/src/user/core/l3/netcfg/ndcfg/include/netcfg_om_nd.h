/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_OM_ND.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/01/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
#ifndef NETCFG_OM_ND_H
#define NETCFG_OM_ND_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "l_sort_lst.h"
#include "netcfg_type.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define NETCFG_OM_ND_MSGBUF_TYPE_SIZE sizeof(union NETCFG_OM_ND_IPCMsg_Type_U)


typedef struct NETCFG_OM_ND_CONFIG_S
{
    
    UI32_T  ns_interval;
    UI32_T  dad_attempts;
    UI32_T  valid_lifetime;
    UI32_T  preferred_lifetime;
    UI32_T reachable_time;
    UI32_T ra_lifetime;
    UI32_T max_ra_interval;
    UI32_T min_ra_interval;
    L_SORT_LST_List_T ra_prefix_list;//mixed with ipv6 and ipv4
    UI16_T flag;
#define NETCFG_OM_ND_FLAG_MANAGED       (1)
#define NETCFG_OM_ND_FLAG_OTHER         (1<<1)
#define NETCFG_OM_ND_FLAG_SUPPRESS      (1<<2)
#define NETCFG_OM_ND_FLAG_ISSET_RA_HOPLIMIT       (1<<3)
#define NETCFG_OM_ND_FLAG_ISSET_REACHABLE_TIME (1<<4)
#define NETCFG_OM_ND_FLAG_ISSET_NS_INTERVAL        (1<<5)
#define NETCFG_OM_ND_FLAG_ISSET_RA_INTERVAL        (1<<6)
#define NETCFG_OM_ND_FLAG_ISSET_RA_LIFETIME         (1<<7)
    UI8_T  ra_hoplimit;
    UI8_T    preference;//NETCFG_RA_ROUTER_PREFERENCE_T
}NETCFG_OM_ND_CONFIG_T;

typedef struct NETCFG_OM_ND_PREFIX_S
{
    UI32_T ifIndex; //key1
    L_INET_AddrIp_T prefix;//key2

    /* Prefix information field. */
    UI32_T  flags;				/* Flags: ONLINK and AUTOADDR*/
    UI32_T vlifetime;			/* Valid lifetime. */
    UI32_T plifetime;			/* Preferred lifetime. */
#define NETCFG_OM_ND_FLAG_ON_LINK       1
#define NETCFG_OM_ND_FLAG_AUTO_ADDRESS  (1<<1)
}NETCFG_OM_ND_PREFIX_T;

typedef struct NETCFG_OM_ND_RourterRedundancyEntry_S
{
    L_INET_AddrIp_T ip_addr;
    UI32_T ifindex;
    UI8_T mac[SYS_ADPT_MAC_ADDR_LEN];
}NETCFG_OM_ND_RouterRedundancyEntry_T;

enum
{
    NETCFG_OM_ND_IPC_GETTIMEOUT,
    NETCFG_OM_ND_IPC_GETNDDADATTEMPTS,
    NETCFG_OM_ND_IPC_GETNDNSINTERVAL,
    NETCFG_OM_ND_IPC_GETNDHOPLIMIT,
    NETCFG_OM_ND_IPC_GETNDPREFIX,
    NETCFG_OM_ND_IPC_GETMANAGEDFLAG,
    NETCFG_OM_ND_IPC_GETOTHERFLAG,
    NETCFG_OM_ND_IPC_GETNDREACHABLETIME,
    NETCFG_OM_ND_IPC_GETRASUPPRESS,
    NETCFG_OM_ND_IPC_GETRALIFETIME,
    NETCFG_OM_ND_IPC_GETRAINTERVAL,
    NETCFG_OM_ND_IPC_GETRAROUTERPREFERENCE,
    NETCFG_OM_ND_IPC_ISCONFIGFLAGSET,
    NETCFG_OM_ND_IPC_GETRPENTRY,
    NETCFG_OM_ND_IPC_GETNEXTSTATICENTRY,
    NETCFG_OM_ND_IPC_RA_GUARD_ISENABLED,
    NETCFG_OM_ND_IPC_RA_GUARD_ISANYPORTENABLED,
    //_GetNdRouterPreference
};


/* Macro function for computation of IPC msg_buf size based on field name
 * used in NETCFG_OM_ND_IPCMsg_T.data
 */
#define NETCFG_OM_ND_GET_MSG_SIZE(field_name)                       \
            (NETCFG_OM_ND_MSGBUF_TYPE_SIZE +                        \
            sizeof(((NETCFG_OM_ND_IPCMsg_T*)0)->data.field_name))

typedef struct
{
    union NETCFG_OM_ND_IPCMsg_Type_U
    {
        UI32_T cmd;            /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result_bool; /*respond bool return*/
        UI32_T result_ui32; /*respond ui32 return*/ 
    } type;

    union
    {
        BOOL_T        bool_v;
        UI32_T        ui32_v;    
        struct
        {
            UI32_T ifindex;
            UI32_T ui32_v;
        } arg_ifindex_and_ui32;
        struct
        {
            UI32_T ifindex;
            UI32_T ui32_1_v;
            UI32_T ui32_2_v;
        } arg_ifindex_and_ui32x2;
        struct
        {
            UI32_T ifindex;
            BOOL_T bool_v;
        } arg_ifindex_and_bool;
        struct
        {
            UI32_T ui32_v1;
            UI32_T ui32_v2;
            BOOL_T bool_v;
        } arg_ui32_ui32_bool;

        struct 
        {
            UI32_T ifIndex;
            L_INET_AddrIp_T prefix;
            /* Prefix information field. */
            UI32_T vlifetime;
            UI32_T plifetime;
            UI8_T  enable_onlink;
            UI8_T  enable_autoaddr;
        }arg_nd_prefix;

        struct
        {
        }arg_empty;

        NETCFG_OM_ND_RouterRedundancyEntry_T arg_rp_entry;
        
        struct
        {
            UI8_T  addr_type;
            NETCFG_TYPE_StaticIpNetToPhysicalEntry_T entry;
        }arg_nd_static_entry;
        
    } data;
}NETCFG_OM_ND_IPCMsg_T;

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
typedef struct NETCFG_OM_ND_RaGuardPortStatisEntry_S
{
    UI32_T  drop_cnt[NETCFG_TYPE_RG_PKT_MAX];     /* drop    count */
    UI32_T  recv_cnt[NETCFG_TYPE_RG_PKT_MAX];     /* receive count */
} NETCFG_OM_ND_RaGuardPortStatisEntry_T;
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for ARP OM.
 *
 * INPUT   : ipcmsg_p -- input request ipc message buffer
 *
 * OUTPUT  : ipcmsg_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_ND_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/* FUNCTION NAME : NETCFG_OM_ND_Init
 * PURPOSE:create semaphore
 *
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void NETCFG_OM_ND_Init(void);

/* for removal warning */
/* FUNCTION NAME : NETCFG_OM_ND_GetStaticEntry
 * PURPOSE:
 *      Get a static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T NETCFG_OM_ND_GetStaticEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ND_GetNextStaticEntry
 * PURPOSE:
 *      Get next available static arp entry.
 *
 * INPUT:
 *      entry
 *      addr_type: must either L_INET_ADDR_TYPE_IPV4 or L_INET_ADDR_TYPE_IPV6
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 *      If key is (0, 0), get first one.
 */
BOOL_T NETCFG_OM_ND_GetNextStaticEntry(UI8_T addr_type,NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ND_DeleteAllStaticEntry
 * PURPOSE:
 *          Remove all static arp entries.
 *
 * INPUT:  None.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES:  None.
 */
void   NETCFG_OM_ND_DeleteAllStaticEntry(void);

/* FUNCTION NAME : NETCFG_OM_ND_AddStaticEntry
 * PURPOSE:
 *      Add an static ARP entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T NETCFG_OM_ND_AddStaticEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry);


/* FUNCTION NAME : NETCFG_OM_ND_DeleteStaticEntry
 * PURPOSE:
 *      Remove a static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T NETCFG_OM_ND_DeleteStaticEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ND_UpdateStaticEntry
 * PURPOSE:
 *      Update a static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T    NETCFG_OM_ND_UpdateStaticEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ND_SetTimeout
 * PURPOSE:
 *      Set arp age timeout.
 *
 * INPUT:
 *      age_time
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 */
BOOL_T NETCFG_OM_ND_SetTimeout(UI32_T age_time);


/* FUNCTION NAME : NETCFG_OM_ND_GetTimeout
 * PURPOSE:
 *      Get arp age timeout.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      age_time.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 */
BOOL_T NETCFG_OM_ND_GetTimeout(UI32_T *age_time);

/* FUNCTION NAME : NETCFG_OM_ND_AddRouterRedundancyProtocolIpNetToPhysicalEntry
 * PURPOSE:
 *      Add a VRRP/HSRP ARP entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is entry->ip_addr.
 */
BOOL_T NETCFG_OM_ND_AddRouterRedundancyProtocolIpNetToPhysicalEntry(NETCFG_OM_ND_RouterRedundancyEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ND_DeleteRouterRedundancyProtocolIpNetToPhysicalEntry
 * PURPOSE:
 *      Delete a VRRP/HSRP ARP entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is entry->ip_addr.
 */
BOOL_T NETCFG_OM_ND_DeleteRouterRedundancyProtocolIpNetToPhysicalEntry(NETCFG_OM_ND_RouterRedundancyEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ND_GetRouterRedundancyProtocolIpNetToPhysicalEntry
 * PURPOSE:
 *      Get a VRRP/HSRP ARP entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is entry->ip_addr.
 */
BOOL_T NETCFG_OM_ND_GetRouterRedundancyProtocolIpNetToPhysicalEntry(NETCFG_OM_ND_RouterRedundancyEntry_T *entry);

#ifdef SYS_CPNT_IPV6

/* FUNCTION NAME : NETCFG_OM_ND_GetNdDADAttempts
 * PURPOSE:
 *    Get DAD ( duplicate address detection) attempts 
 * INPUT:
 *    attempts.
 *
 * OUTPUT:
 *    attempts.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_SetNdDADAttempts(UI32_T vid_ifIndex,UI32_T  attempts);
BOOL_T NETCFG_OM_ND_GetNdDADAttempts(UI32_T vid_ifIndex,UI32_T *attempts);

/* FUNCTION NAME : NETCFG_OM_ND_GetNdNsInterval
 * PURPOSE:
 *    get  the interval between IPv6 neighbor solicitation retransmissions on an interface 
 * INPUT:
 *    vid_ifIndex
 *    msec.
 *
 * OUTPUT:
 *    msec.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_SetNdNsInterval(UI32_T vid_ifIndex, UI32_T msec);
BOOL_T NETCFG_OM_ND_UnsetNdNsInterval(UI32_T vid_ifIndex);
BOOL_T NETCFG_OM_ND_GetNdNsInterval(UI32_T vid_ifIndex, UI32_T *msec);
BOOL_T NETCFG_OM_ND_IsSetNdNsInterval(UI32_T vid_ifIndex);
/* FUNCTION NAME : NETCFG_OM_ND_GetNdHoplimit
 * PURPOSE:
 *    get  the maximum number of hops used in router advertisements 
 * INPUT:
 *    vid_ifIndex
 *    hoplimit.
 *
 * OUTPUT:
 *    hoplimit.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_SetGlobalNdHoplimit(UI32_T hoplimit);
BOOL_T NETCFG_OM_ND_SetNdHoplimit(UI32_T vid_ifIndex,UI32_T hoplimit);
BOOL_T NETCFG_OM_ND_UnsetGlobalNdHoplimit();
BOOL_T NETCFG_OM_ND_UnsetNdHoplimit(UI32_T vid_ifIndex);
BOOL_T NETCFG_OM_ND_GetGlobalNdHoplimit(UI32_T *hoplimit);
BOOL_T NETCFG_OM_ND_GetNdHoplimit(UI32_T vid_ifIndex,UI32_T *hoplimit);
BOOL_T NETCFG_OM_ND_IsSetGlobalNdHoplimit();
BOOL_T NETCFG_OM_ND_IsSetNdHoplimit(UI32_T vid_ifIndex);


/* FUNCTION NAME : NETCFG_OM_ND_GetNdPrefix
 * PURPOSE:
 *    get  which IPv6 prefixes are included in IPv6 router advertisements
 * INPUT:
 *     L_INET_AddrIp_T: prefix
 *    UI32_T vid_ifIndex ,  validLifetime, preferredLifetime
 *    BOOL_T: enable_on_link. enable_autoconf
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_SetNdPrefix(UI32_T vid_ifIndex, L_INET_AddrIp_T *prefix, UI32_T  validLifetime, UI32_T  preferredLifetime,BOOL_T enable_on_link,BOOL_T enable_autoconf);
BOOL_T NETCFG_OM_ND_UnsetNdPrefix(UI32_T vid_ifIndex, L_INET_AddrIp_T *prefix);


/* FUNCTION NAME : NETCFG_OM_ND_GetNdPrefix
 * PURPOSE:
 *    get  which IPv6 prefixes   included in IPv6 router advertisements
 * INPUT:
 *     vid_ifIndex
 *
 * OUTPUT:
 *     L_INET_AddrIp_T: prefix
 *    UI32_T  validLifetime, preferredLifetime
 *    BOOL_T: offLink. noAutoconfig
 *
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_GetNdPrefix(UI32_T vid_ifIndex, L_INET_AddrIp_T *prefix, UI32_T *validLifetime, UI32_T *preferredLifetime,BOOL_T *enable_on_link, BOOL_T *enable_autoconf);
/*input: vidifindex*/
BOOL_T NETCFG_OM_ND_GetNextNdPrefix(UI32_T vid_ifIndex, L_INET_AddrIp_T *prefix, UI32_T *validLifetime, UI32_T *preferredLifetime,BOOL_T *enable_on_link, BOOL_T *enable_autoconf);

/* FUNCTION NAME : NETCFG_OM_ND_GetNdManagedConfigFlag
 * PURPOSE:
 *    get    the "managed address configuration" flag in IPv6 router advertisements
 * INPUT:
 *    vid_ifIndex
 *    enableFlag.
 *
 * OUTPUT:
 *    enableFlag.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_SetNdManagedConfigFlag(UI32_T vid_ifIndex, BOOL_T  enableFlag);
BOOL_T NETCFG_OM_ND_GetNdManagedConfigFlag(UI32_T vid_ifIndex, BOOL_T *enableFlag);

/* FUNCTION NAME : NETCFG_OM_ND_GetNdOtherConfigFlag
 * PURPOSE:
 *    get    the "other stateful configuration" flag in IPv6 router advertisements
 * INPUT:
 *    vid_ifIndex
 *    enableFlag.
 *
 * OUTPUT:
 *    enableFlag.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_SetNdOtherConfigFlag(UI32_T vid_ifIndex, BOOL_T  enableFlag);
BOOL_T NETCFG_OM_ND_GetNdOtherConfigFlag(UI32_T vid_ifIndex, BOOL_T *enableFlag);

/* FUNCTION NAME : NETCFG_OM_ND_GetNdReachableTime
 * PURPOSE:
 *    get    the amount of time that a remote IPv6 node is considered reachable  
 *                  after some reachability confirmation event has occurred
 * INPUT:
 *    vid_ifIndex
 *    msec.
 *
 * OUTPUT:
 *    msec.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_SetNdReachableTime(UI32_T vid_ifIndex,UI32_T  msec);
BOOL_T NETCFG_OM_ND_UnsetNdReachableTime(UI32_T vid_ifIndex);
BOOL_T NETCFG_OM_ND_GetNdReachableTime(UI32_T vid_ifIndex,UI32_T *msec);
BOOL_T NETCFG_OM_ND_IsSetNdReachableTime(UI32_T vid_ifIndex);


/* FUNCTION NAME : NETCFG_OM_ND_GetNdRaSuppress
 * PURPOSE:
 *    get whether suppress  IPv6 router advertisement transmissions
 * INPUT:
 *    vid_ifIndex
 *    enableSuppress.
 *
 * OUTPUT:
 *    enableSuppress.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_SetNdRaSuppress(UI32_T vid_ifIndex, BOOL_T  enableSuppress);
BOOL_T NETCFG_OM_ND_GetNdRaSuppress(UI32_T vid_ifIndex, BOOL_T *enableSuppress);

/* FUNCTION NAME : NETCFG_OM_ND_GetNdRaLifetime
 * PURPOSE:
 *    get the router lifetime value in IPv6 router advertisements 
 * INPUT:
 *    vid_ifIndex
 *    seconds.
 *
 * OUTPUT:
 *    seconds.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_SetNdRaLifetime(UI32_T vid_ifIndex, UI32_T  seconds);
BOOL_T NETCFG_OM_ND_GetNdRaLifetime(UI32_T vid_ifIndex, UI32_T *seconds);
BOOL_T NETCFG_OM_ND_UnsetNdRaLifetime(UI32_T vid_ifIndex);
BOOL_T NETCFG_OM_ND_IsSetNdRaLifetime(UI32_T vid_ifIndex);
/* FUNCTION NAME : NETCFG_OM_ND_GetNdRaInterval
 * PURPOSE:
 *    get the interval between IPv6 router advertisement  transmissions
 * INPUT:
 *    vid_ifIndex  -- vlan ifindex
 *    max          -- max ra interval
 *    min          -- min ra interval
 *
 * OUTPUT:
 *    seconds.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_SetNdRaInterval(UI32_T vid_ifIndex, UI32_T max, UI32_T min);
BOOL_T NETCFG_OM_ND_GetNdRaInterval(UI32_T vid_ifIndex, UI32_T *max_p, UI32_T *min_p);
BOOL_T NETCFG_OM_ND_UnsetNdRaInterval(UI32_T vid_ifIndex);
BOOL_T NETCFG_OM_ND_IsSetNdRaInterval(UI32_T vid_ifIndex);
/* FUNCTION NAME : NETCFG_OM_ND_GetNdRouterPreference
 * PURPOSE:
 *    get the the Preference flag in IPv6 router advertisements
 * INPUT:
 *    vid_ifIndex
 *    prefer.
 *
 * OUTPUT:
 *    prefer. type of NETCFG_TYPE_ND_ROUTER_PREFERENCE_E
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_SetNdRouterPreference(UI32_T vid_ifIndex, UI32_T   prefer);
BOOL_T NETCFG_OM_ND_GetNdRouterPreference(UI32_T vid_ifIndex, UI32_T  *prefer);

/* FUNCTION NAME : NETCFG_OM_ND_InitNdConfig
 * PURPOSE:
 *    clear (reset ) ND configuration on a vlan or tunnel interface
 * INPUT:
 *    vid_ifIndex
 *     
 *
 * OUTPUT:
 *     
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_OM_ND_InitNdConfig(UI32_T vid_ifIndex);

BOOL_T NETCFG_OM_ND_IsConfigFlagSet(UI32_T vid_ifindex, UI32_T flag, BOOL_T *is_set);

#endif /*SYS_CPNT_IPV6*/


/*for backdoor only*/
void NETCFG_OM_ND_BackdoorSetDebugFlag(UI32_T flag);
void NETCFG_OM_ND_BackdoorDumpDB(UI32_T vid_ifIndex);

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_RAGUARD_ClearOm
 * ------------------------------------------------------------------------
 * PURPOSE: To clear RA Guard OM.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void NETCFG_OM_ND_RAGUARD_ClearOm(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_RAGUARD_IsAnyPortEnabled
 * ------------------------------------------------------------------------
 * PURPOSE: To check if RA Guard is enabled for any port.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_ND_RAGUARD_IsAnyPortEnabled(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_RAGUARD_IsEnabled
 * ------------------------------------------------------------------------
 * PURPOSE: To check if RA Guard is enabled for specifed lport.
 * INPUT  : lport    - which lport to check (1-based)
 *          pkt_type - which packet type received,
 *                     NETCFG_TYPE_RG_PKT_MAX to skip statistics
 *                     (NETCFG_TYPE_RG_PKT_RA/NETCFG_TYPE_RG_PKT_RR)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_ND_RAGUARD_IsEnabled(
    UI32_T  lport,
    UI32_T  pkt_type);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_RAGUARD_SetPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To set RA Guard port status for specifed lport.
 * INPUT  : lport     - which lport to set (1-based)
 *          is_enable - TRUE to enable
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_ND_RAGUARD_SetPortStatus(
    UI32_T  lport,
    BOOL_T  is_enable);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_RAGUARD_GetPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To get RA Guard port status for specifed lport.
 * INPUT  : lport       - which lport to get (1-based)
 * OUTPUT : is_enable_p - pointer to output status
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_ND_RAGUARD_GetPortStatus(
    UI32_T  lport,
    BOOL_T  *is_enable_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_RAGUARD_CopyPortCfgTo
 * ------------------------------------------------------------------------
 * PURPOSE: To copy port status from specified src lport to dst lprot.
 * INPUT  : src_lport - which src lport to copy from (1-based)
 *          dst_lport - which dst lport to copy to   (1-based)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_ND_RAGUARD_CopyPortCfgTo(
    UI32_T  src_lport,
    UI32_T  dst_lport);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_RAGUARD_GetStatistics
 * ------------------------------------------------------------------------
 * PURPOSE: To get statistics structure for specified lport.
 * INPUT  : lport   - which lport to get (1-based)
 * OUTPUT : pstat_p - pointer to output statistics structure
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_ND_RAGUARD_GetStatistics(
    UI32_T                                  lport,
    NETCFG_OM_ND_RaGuardPortStatisEntry_T   *pstat_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_RAGUARD_IncStatistics
 * ------------------------------------------------------------------------
 * PURPOSE: To increment the statistics for specifed lport.
 * INPUT  : lport    - which lport to inc (1-based)
 *          pkt_type - which type to inc
 *                     (NETCFG_TYPE_RG_PKT_RA/NETCFG_TYPE_RG_PKT_RR)
 *          recv_cnt - receive count to inc
 *          drop_cnt - drop count to inc
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void NETCFG_OM_ND_RAGUARD_IncStatistics(
    UI32_T  lport,
    UI32_T  pkt_type,
    UI32_T  recv_cnt,
    UI32_T  drop_cnt);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_RAGUARD_BackdoorDumpDB
 * ------------------------------------------------------------------------
 * PURPOSE: To dump RA Guard OM for backdoor.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void NETCFG_OM_ND_RAGUARD_BackdoorDumpDB(void);

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

#endif /*NETCFG_OM_ND_H*/

