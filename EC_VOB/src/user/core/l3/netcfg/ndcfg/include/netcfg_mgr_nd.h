/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_MGR_ND.H
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
#ifndef NETCFG_MGR_ND_H
#define NETCFG_MGR_ND_H

#include <sys_type.h>
#include "sysfun.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "netcfg_type.h"

#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
#include "l_mm.h"
#endif

#define NETCFG_MGR_ND_IPCMSG_TYPE_SIZE sizeof(union NETCFG_MGR_ND_IPCMsg_Type_U)

#define NETCFG_MGR_ND_MAX_RA_INTERVAL_RANGE_MAX     1800
#define NETCFG_MGR_ND_MAX_RA_INTERVAL_RANGE_MIN     4
#define NETCFG_MGR_ND_MIN_RA_INTERVAL_RANGE_MIN	    3
#define NETCFG_MGR_ND_RA_LIFETIME_RANGE_MAX         9000

/* command used in IPC message
 */
enum
{
    NETCFG_MGR_ND_IPC_CREATESTATIC,
    NETCFG_MGR_ND_IPC_DELETESTATIC,
    NETCFG_MGR_ND_IPC_SETSTATICND,
    NETCFG_MGR_ND_IPC_GETNDENTRY,
    NETCFG_MGR_ND_IPC_GETNEXTNDENTRY,
    NETCFG_MGR_ND_IPC_GETNEXTIPV4NDENTRY,
    NETCFG_MGR_ND_IPC_GETNEXTIPV6NDENTRY,
    NETCFG_MGR_ND_IPC_DELETEALLDYNAMIC,
    NETCFG_MGR_ND_IPC_DELETEALLDYNAMICIPV4,
    NETCFG_MGR_ND_IPC_DELETEALLDYNAMICIPV6,
    NETCFG_MGR_ND_IPC_SETTIMEOUT,
    NETCFG_MGR_ND_IPC_GETSTATISTICS,
    NETCFG_MGR_ND_IPC_GETRUNNINGTIMEOUT,
    NETCFG_MGR_ND_IPC_GETNEXTSTATICNDENTRY,
    NETCFG_MGR_ND_IPC_GETNEXTSTATICIPV4NDENTRY,
    NETCFG_MGR_ND_IPC_GETNEXTSTATICIPV6NDENTRY,
    NETCFG_MGR_ND_IPC_SIGNALRIFUP,
    NETCFG_MGR_ND_IPC_SIGNALRIFDOWN,
    NETCFG_MGR_ND_IPC_SIGNALRIFCREATE,
    NETCFG_MGR_ND_IPC_SIGNALRIFDESTORY,
    NETCFG_MGR_ND_IPC_SETDADATTEMPTS,
    NETCFG_MGR_ND_IPC_GETRUNNINGDADATTEMPTS,
    NETCFG_MGR_ND_IPC_SETNDNSINTERVAL,
    NETCFG_MGR_ND_IPC_UNSETNDNSINTERVAL,
    NETCFG_MGR_ND_IPC_GETRUNNINGNDNSINTERVAL,
    NETCFG_MGR_ND_IPC_SETNDHOPLIMIT,
    NETCFG_MGR_ND_IPC_UNSETNDHOPLIMIT,
    NETCFG_MGR_ND_IPC_GETRUNNINGNDHOPLIMIT,
    NETCFG_MGR_ND_IPC_SETNDPREFIX,
    NETCFG_MGR_ND_IPC_UNSETNDPREFIX,
    NETCFG_MGR_ND_IPC_GETRUNNINGNEXTNDPREFIX,
    NETCFG_MGR_ND_IPC_SETNDMANAGEDCONFIG,
    NETCFG_MGR_ND_IPC_GETRUNNINGNDMANAGEDCONFIG,
    NETCFG_MGR_ND_IPC_SETNDOTHERCONFIG,
    NETCFG_MGR_ND_IPC_GETRUNNINGNDOTHERCONFIG,
    NETCFG_MGR_ND_IPC_SETNDREACHABLETIME,
    NETCFG_MGR_ND_IPC_UNSETNDREACHABLETIME,
    NETCFG_MGR_ND_IPC_GETRUNNINGNDREACHABLETIME,
    NETCFG_MGR_ND_IPC_SETNDRASUPPRESS,
    NETCFG_MGR_ND_IPC_GETRUNNINGNDRASUPPRESS,
    NETCFG_MGR_ND_IPC_SETNDRALIFETIME,
    NETCFG_MGR_ND_IPC_UNSETNDRALIFETIME,
    NETCFG_MGR_ND_IPC_GETRUNNINGNDRALIFETIME,
    NETCFG_MGR_ND_IPC_SETNDRAINTERVAL,
    NETCFG_MGR_ND_IPC_UNSETNDRAINTERVAL,
    NETCFG_MGR_ND_IPC_GETRUNNINGNDRAINTERVAL,
    NETCFG_MGR_ND_IPC_SETNDROUTERPREFERENCE,
    NETCFG_MGR_ND_IPC_GETRUNNINGNDROUTERPREFERENCE,
    NETCFG_MGR_ND_IPC_ADDROUTERREDUNDANCYPROTOCOLENTRY,
    NETCFG_MGR_ND_IPC_DELETEROUTERREDUNDANCYPROTOCOLENTRY,
    NETCFG_MGR_ND_IPC_RAGUARD_GETPORTSTATUS,
    NETCFG_MGR_ND_IPC_RAGUARD_GETNEXTPORTSTATUS,
    NETCFG_MGR_ND_IPC_RAGUARD_SETPORTSTATUS,
    NETCFG_MGR_ND_IPC_RAGUARD_GETRUNNINGPORTSTATUS,

};

/* Macro function for computation of IPC msg_buf size based on field name
 * used in NETCFG_MGR_ND_IPCMsg_T.data
 */
#define NETCFG_MGR_ND_GET_MSG_SIZE(field_name)                       \
            (NETCFG_MGR_ND_IPCMSG_TYPE_SIZE +                        \
            sizeof(((NETCFG_MGR_ND_IPCMsg_T*)0)->data.field_name))



typedef struct
{
    union NETCFG_MGR_ND_IPCMsg_Type_U
    {
        UI32_T cmd;                /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result_bool;      /*respond bool return*/
        UI32_T result_ui32;      /*respond ui32 return*/
        SYS_TYPE_Get_Running_Cfg_T         result_running_cfg; /* For running config API */
    } type;

    union
    {
        UI32_T        ui32_v;
        NETCFG_TYPE_IpNetToPhysicalEntry_T           arg_nd_entry; /*all type ND entry*/
        NETCFG_TYPE_StaticIpNetToPhysicalEntry_T       arg_nd_static_entry;/*static ND entry*/
        NETCFG_TYPE_IpNetToMedia_Statistics_T    arg_nd_stat;
        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            UI32_T arg3;
            UI8_T  arg4[NETCFG_TYPE_PHY_ADDRESEE_LENGTH];
        } arg_grp1;


        struct
        {
            UI32_T arg1;
            UI32_T arg2;
        } arg_grp2;

        struct
        {
            NETCFG_TYPE_StaticIpNetToMediaEntry_T arg1;
            UI32_T arg2;
        } arg_grp3;
        struct
        {
            NETCFG_TYPE_StaticIpNetToPhysicalEntry_T       static_entry;
            UI32_T ui32_v;
        }arg_static_entry_and_ui32;

        struct
        {
            UI32_T     vid_ifindex;
            UI32_T     ui32_v;
        } arg_ifindex_and_ui32;
        struct
        {
            UI32_T     vid_ifindex;
            UI32_T     ui32_1_v;
            UI32_T     ui32_2_v;
        } arg_ifindex_and_ui32x2;
        struct
        {
            UI32_T     vid_ifindex;
            BOOL_T     bool_v;
        } arg_ifindex_and_bool;

        struct
        {
            UI32_T vid_ifindex;
            L_INET_AddrIp_T prefix;
            UI32_T valid_lifetime;
            UI32_T preferred_lifetime;
            BOOL_T enable_onlink;
            BOOL_T enable_autoconf;
        }arg_nd_prefix;

        struct
        {
            UI32_T vid_ifindex;
            L_INET_AddrIp_T ip_addr;
            NETCFG_TYPE_PhysAddress_T phy_addr;
        } arg_nd_config;
    } data;
}NETCFG_MGR_ND_IPCMsg_T;

/* FUNCTION NAME : NETCFG_MGR_ND_InitiateProcessResources
 * PURPOSE:
 *      Initialize NETCFG_MGR_ND used system resource, eg. protection semaphore.
 *      Clear all working space.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  - Success
 *      FALSE - Fail
 */
BOOL_T NETCFG_MGR_ND_InitiateProcessResources(void);

/* FUNCTION NAME : NETCFG_MGR_ND_Create_InterCSC_Relation
 * PURPOSE:
 *      This function initializes all function pointer registration operations.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 */
void NETCFG_MGR_ND_Create_InterCSC_Relation(void);

/* FUNCTION NAME : NETCFG_MGR_ND_EnterMasterMode
 * PURPOSE:
 *      Make Routing Engine enter master mode, handling all TCP/IP configuring requests,
 *      and receiving packets.
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
 *
 */
void NETCFG_MGR_ND_EnterMasterMode (void);

/* FUNCTION NAME : NETCFG_MGR_ND_ProvisionComplete
 * PURPOSE:
 *      1. Let default gateway CFGDB into route when provision complete.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 *
 * NOTES:
 *
 */
void NETCFG_MGR_ND_ProvisionComplete(void);

/* FUNCTION NAME : NETCFG_MGR_ND_EnterSlaveMode
 * PURPOSE:
 *      Make Routing Engine enter slave mode, discarding all TCP/IP configuring requests,
 *      and receiving packets.
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
 *      1. In slave mode, just rejects function request and discard incoming message.
 */
void NETCFG_MGR_ND_EnterSlaveMode (void);

/* FUNCTION NAME : NETCFG_MGR_ND_SetTransitionMode
 * PURPOSE:
 *      Make Routing Engine enter transition mode, releasing all allocateing resource in master mode,
 *      discarding TCP/IP configuring requests, and receiving packets.
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
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_ND_SetTransitionMode(void);

/* FUNCTION NAME : NETCFG_MGR_ND_EnterTransitionMode
 * PURPOSE:
 *      Make Routing Engine enter transition mode, releasing all allocateing resource in master mode,
 *      discarding TCP/IP configuring requests, and receiving packets.
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
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_ND_EnterTransitionMode (void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for ND MGR.
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
BOOL_T NETCFG_MGR_ND_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_AddStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a static ND entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_AddStaticIpNetToPhysicalEntry(UI32_T vid_ifindex,
                                                                         L_INET_AddrIp_T* ip_addr,
                                                                         UI32_T phy_addr_len,
                                                                         UI8_T *phy_addr);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_DeleteStaticIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a static ND entry.
 *
 * INPUT   : vid_ifindex,
 *              ip_addr,
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_DeleteStaticIpNetToPhysicalEntry(UI32_T vid_ifindex, L_INET_AddrIp_T* ip_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_GetNextStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE :Lookup next static ND entry.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_GetNextStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);
//action_flags: possible action_flags are ND_MGR_GET_FLAGS_IPV4 and ND_MGR_GET_FLAGS_IPV6
UI32_T NETCFG_MGR_ND_GetNextStaticIpNetToPhysicalEntry(UI32_T action_flags, NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_SetIpNetToMediaTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE : Set ND timeout.
 *
 * INPUT   : age_time.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_SetIpNetToPhysicalTimeout(UI32_T age_time);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_AddRouterRedundancyProtocolIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a VRRP/HSRP ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_AddRouterRedundancyProtocolIpNetToPhysicalEntry(UI32_T vid_ifindex,
                                                                         L_INET_AddrIp_T* ip_addr,
                                                                         UI32_T phy_addr_len,
                                                                         UI8_T *phy_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_DeleteRouterRedundancyProtocolIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Remove a VRRP/HSRP ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_DeleteRouterRedundancyProtocolIpNetToPhysicalEntry(UI32_T vid_ifindex,
                                                                         L_INET_AddrIp_T* ip_addr,
                                                                         UI32_T phy_addr_len,
                                                                         UI8_T *phy_addr);

#if (SYS_CPNT_IPV6 == TRUE)
UI32_T NETCFG_MGR_ND_SetIpv6NetToPhysicalTimeout(UI32_T age_time);
#endif
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_DeleteAllDynamicIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic ND entries.
 *
 * INPUT   : action flag: ND_MGR_GET_FLAGS_IPV4,  ND_MGR_GET_FLAGS_IPV6, or both
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_DeleteAllDynamicIpNetToPhysicalEntry(UI32_T actionflags);


BOOL_T NETCFG_MGR_ND_GetIpNetToPhysicalEntry(NETCFG_TYPE_IpNetToPhysicalEntry_T *entry);
UI32_T NETCFG_MGR_ND_SetStaticIpNetToPhysicalEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry, int type);


#if (SYS_CPNT_IPV6 == TRUE)
UI32_T NETCFG_MGR_ND_SetNdDADAttempts(UI32_T vid_ifindex, UI32_T attempts);
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningDADAttempts(UI32_T vid_ifindex, UI32_T* attempts);

UI32_T NETCFG_MGR_ND_SetNdNsInterval(UI32_T vid_ifindex, UI32_T msec);
UI32_T NETCFG_MGR_ND_UnsetNdNsInterval(UI32_T vid_ifindex);
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdNsInterval(UI32_T vid_ifindex, UI32_T* msec);

UI32_T NETCFG_MGR_ND_SetNdReachableTime(UI32_T vid_ifindex,UI32_T msec);
UI32_T NETCFG_MGR_ND_UnsetNdReachableTime(UI32_T vid_ifindex);
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdReachableTime(UI32_T vid_ifindex, UI32_T* msec);

#if (SYS_CPNT_IPV6_ROUTING == TRUE)
UI32_T NETCFG_MGR_ND_SetNdHoplimit(UI32_T hoplimit);
UI32_T NETCFG_MGR_ND_UnsetNdHoplimit();
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdHoplimit(UI32_T* hoplimit);

UI32_T NETCFG_MGR_ND_SetNdPrefix(UI32_T vid_ifindex, L_INET_AddrIp_T* prefix, UI32_T validLifetime, UI32_T preferredLifetime,BOOL_T enable_onlink,BOOL_T enable_autoconf);
UI32_T NETCFG_MGR_ND_UnSetNdPrefix(UI32_T vid_ifindex, L_INET_AddrIp_T* prefix);
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNextNdPrefix(UI32_T vid_ifindex, L_INET_AddrIp_T* prefix, UI32_T *valid_lifetime, UI32_T *preferred_lifetime,BOOL_T*enable_on_link,BOOL_T* enable_autoconf);

UI32_T NETCFG_MGR_ND_SetNdManagedConfigFlag(UI32_T vid_ifindex, BOOL_T enableFlag);
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdManagedConfigFlag(UI32_T vid_ifindex, BOOL_T* enableFlag);

UI32_T NETCFG_MGR_ND_SetNdOtherConfigFlag(UI32_T vid_ifindex, BOOL_T enableFlag);
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdOtherConfigFlag(UI32_T vid_ifindex, BOOL_T* enableFlag);

UI32_T NETCFG_MGR_ND_SetNdRaSuppress(UI32_T vid_ifindex, BOOL_T enableSuppress);
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdRaSuppress(UI32_T vid_ifindex, BOOL_T* enableSuppress);

UI32_T NETCFG_MGR_ND_SetNdRaLifetime(UI32_T vid_ifindex, UI32_T seconds);
UI32_T NETCFG_MGR_ND_UnsetNdRaLifetime(UI32_T vid_ifindex);
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdRaLifetime(UI32_T vid_ifindex, UI32_T* seconds);

UI32_T NETCFG_MGR_ND_SetNdRaInterval(UI32_T vid_ifindex, UI32_T max, UI32_T min);
UI32_T NETCFG_MGR_ND_UnsetNdRaInterval(UI32_T vid_ifindex);
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdRaInterval(UI32_T vid_ifindex, UI32_T *max_p, UI32_T *min_p);

UI32_T NETCFG_MGR_ND_SetNdRouterPreference(UI32_T vid_ifindex, UI32_T  prefer);
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdRouterPreference(UI32_T vid_ifindex, UI32_T*  prefer);

#endif /* #if (SYS_CPNT_IPV6_ROUTING == TRUE) */
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */


////////////////////


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_GetIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Get an ND entry information.
 *
 * INPUT   : entry.
 *              action_flags: possible action_flags are ND_MGR_GET_FLAGS_IPV4 and ND_MGR_GET_FLAGS_IPV6
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :key are ifindex and ip address.
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_GetNextIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry);
UI32_T NETCFG_MGR_ND_GetNextIpNetToPhysicalEntry(UI32_T action_flags, NETCFG_TYPE_IpNetToPhysicalEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_GetStatistics
 *-----------------------------------------------------------------------------
 * PURPOSE : Get ND packet statistics.
 *
 * INPUT   : None.
 *
 * OUTPUT  : stat.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_GetStatistics(NETCFG_TYPE_IpNetToMedia_Statistics_T *stat);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_GetRunningIpNetToPhysicalTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND timeout.
 *
 * INPUT   : None.
 *
 * OUTPUT  : age_time.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_MGR_ND_GetRunningIpNetToPhysicalTimeout(UI32_T *age_time);

/* FUNCTION NAME : NETCFG_MGR_ND_SignalL3IfUp
 * PURPOSE:
 *      Singal that an L3If is up.
 *
 * INPUT:
 *      ifindex     -- the ifindex of the interface
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_MGR_ND_SignalL3IfUp(UI32_T ifindex);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_SignalRifUp
 *-----------------------------------------------------------------------------
 * PURPOSE : when a rif up signal ND .
 *
 * INPUT   : ifindex  -- the ifindex of the interface where ip_addr located
 *           ip_addr  -- the ip_address that is up
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK
 *           NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_SignalRifUp(UI32_T ifindex, L_INET_AddrIp_T* ip_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_SignalRifDown
 *-----------------------------------------------------------------------------
 * PURPOSE : when a rif down signal ND .
 *
 * INPUT   : ifindex  -- the ifindex of the interface where ip_addr located
 *           ip_addr  -- the ip_address that is down
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK
 *           NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_SignalRifDown(UI32_T ifindex, L_INET_AddrIp_T* ip_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_SignalRifCreate
 *-----------------------------------------------------------------------------
 * PURPOSE : when create a rif signal ND .
 *
 * INPUT   : ip_addr, ip_mask.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_SignalRifCreate(L_INET_AddrIp_T* ip_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_SignalRifDestroy
 *-----------------------------------------------------------------------------
 * PURPOSE : when destroy a rif signal ND .
 *
 * INPUT   : ip_addr, ip_mask.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_SignalRifDestroy(L_INET_AddrIp_T* ip_addr);

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_RAGUARD_AddFstTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the port joins the trunk
 *          as the 1st member
 * INPUT  : trunk_ifindex  - specify which trunk to join.
 *          member_ifindex - specify which member port being add to trunk.
 * OUTPUT : None
 * RETURN : None
 * NOTES  : member_ifindex is sure to be a normal port asserted by SWCTRL.
 * ------------------------------------------------------------------------
 */
void NETCFG_MGR_ND_RAGUARD_AddFstTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_RAGUARD_AddTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the 2nd or the following
 *          trunk member is removed from the trunk
 * INPUT  : trunk_ifindex  - specify which trunk to join to
 *          member_ifindex - specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void NETCFG_MGR_ND_RAGUARD_AddTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_RAGUARD_DelLstTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the last trunk member
 *          is removed from the trunk
 * INPUT  : trunk_ifindex   -- specify which trunk to join to
 *          member_ifindex  -- specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void NETCFG_MGR_ND_RAGUARD_DelLstTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_RAGUARD_DelTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the 2nd or the following
 *          trunk member is removed from the trunk
 * INPUT  : trunk_ifindex   -- specify which trunk to join to
 *          member_ifindex  -- specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void NETCFG_MGR_ND_RAGUARD_DelTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_RAGUARD_GetPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To get RA Guard port status for specified lport.
 * INPUT  : lport       - which lport to get
 * OUTPUT : is_enable_p - pointer to output status
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_MGR_ND_RAGUARD_GetPortStatus(
    UI32_T  lport,
    BOOL_T  *is_enable_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_RAGUARD_GetNextPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To get RA Guard next port status for specified lport.
 * INPUT  : lport_p     - which lport to get next
 * OUTPUT : lport_p     - next lport
 *          is_enable_p - pointer to output status
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_MGR_ND_RAGUARD_GetNextPortStatus(
    UI32_T  *lport_p,
    BOOL_T  *is_enable_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_RAGUARD_GetRunningPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To get RA Guard running port status for specified lport.
 * INPUT  : lport       - which lport to get
 * OUTPUT : is_enable_p - pointer to output status
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_RAGUARD_GetRunningPortStatus(
    UI32_T  lport,
    BOOL_T  *is_enable_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_RAGUARD_SetPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To set RA Guard port status for specified lport.
 * INPUT  : lport     - which lport to set
 *          is_enable - TRUE to enable
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_MGR_ND_RAGUARD_SetPortStatus(
    UI32_T  lport,
    BOOL_T  is_enable);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_RAGUARD_HandleHotInsertion
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a new dut insertion of NETCFG_MGR_ND_RAGUARD.
 *
 * INPUT:
 *    starting_port_ifindex  -- starting port ifindex
 *    number_of_port         -- number of ports
 *    use_default            -- whether use default setting
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void NETCFG_MGR_ND_RAGUARD_HandleHotInsertion(
    UI32_T  starting_port_ifindex,
    UI32_T  number_of_port,
    BOOL_T  use_default);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_ND_RAGUARD_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void NETCFG_MGR_ND_RAGUARD_HandleHotRemoval(
    UI32_T  starting_port_ifindex,
    UI32_T  number_of_port);

#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_RAGUARD_ProcessRcvdPDU
 * ------------------------------------------------------------------------
 * PURPOSE: To process received pdu.
 * INPUT  : mref_handle_p - pointer to ingress mref
 *          src_mac_p     - pointer to src mac
 *          dst_mac_p     - pointer to dst mac
 *          ing_vid       - ingress vid
 *          ing_cos       - ingress cos
 *          pkt_type      - packet type (NETCFG_TYPE_RG_PKT_RA/
 *                                       NETCFG_TYPE_RG_PKT_RR)
 *          pkt_length    - length of packet
 *          src_lport     - source lport
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void NETCFG_MGR_ND_RAGUARD_ProcessRcvdPDU(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac_p,
    UI8_T               *src_mac_p,
    UI16_T              ing_vid,
    UI8_T               ing_cos,
    UI8_T               pkt_type,
    UI32_T              pkt_length,
    UI32_T              src_lport);

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

#endif    /* End of NETCFG_MGR_ND_H */

