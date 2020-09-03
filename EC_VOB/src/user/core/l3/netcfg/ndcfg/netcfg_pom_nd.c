/* MODULE NAME:  netcfg_pom_arp.c
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access NETCFG_OM_ND service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call NETCFG_POM_XXX for APIs NETCFG_OM_XXX provided by NETCFG, and same as NETCFG_PMGR_ARP for
 *    NETCFG_MGR_ARP APIs
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    2008/01/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sys_module.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "netcfg_type.h"
#include "netcfg_om_nd.h"
#include "netcfg_pom_nd.h"
#include "sys_dflt.h"



/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T netcfg_om_nd_ipcmsgq_handle;

#define NETCFG_POM_DECLARE_MSG_P(data_type) \
    const UI32_T msg_size = NETCFG_OM_ND_GET_MSG_SIZE(data_type);\
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];\
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;\
    NETCFG_OM_ND_IPCMsg_T *msg_p = (NETCFG_OM_ND_IPCMsg_T*)msgbuf_p->msg_buf;

#define NETCFG_POM_SEND_WAIT_MSG_P(result) \
do{\
    msgbuf_p->cmd = SYS_MODULE_NDCFG;\
    msgbuf_p->msg_size = msg_size;\
    result = SYSFUN_SendRequestMsg(netcfg_om_nd_ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,msg_size, msgbuf_p);\
}while(0)
static L_INET_AddrIp_T NETCFG_POM_ND_ComposeInetAddr(UI16_T type,UI8_T* addrp,UI16_T prefixLen);

/* FUNCTION NAME : NETCFG_POM_ARP_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource for CSCA_POM in the calling process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    Before other CSC use NETCFG_POM_ARP, it should initiate the resource (get the message queue handler internally)
 
 *
 */
BOOL_T NETCFG_POM_ND_InitiateProcessResource(void)
{
    /* Given that CSCA is run in XXX_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_NETCFG_PROC_OM_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &netcfg_om_nd_ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME : NETCFG_POM_ARP_GetIpNetToMediaTimeout
 * PURPOSE:
 *    Get ARP timeout
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    age_time.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetIpNetToMediaTimeout(UI32_T *age_time)
{
    const UI32_T msg_size = NETCFG_OM_ND_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_ND_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_NDCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_ND_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETTIMEOUT;

    if (SYSFUN_SendRequestMsg(netcfg_om_nd_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *age_time = msg_p->data.ui32_v;
    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_POM_ND_GetRunningIpNetToMediaTimeout
 * PURPOSE:
 *    Get running config of ARP timeout
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    age_time.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ND_GetRunningIpNetToMediaTimeout(UI32_T *age_time)
{
    if ( TRUE == NETCFG_POM_ND_GetIpNetToMediaTimeout(age_time))
    {
        if (*age_time != SYS_DFLT_IP_NET_TO_MEDIA_ENTRY_TIMEOUT)
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

/* FUNCTION NAME : NETCFG_POM_ND_GetRouterRedundancyProtocolIpNetToPhysicalEntry
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
BOOL_T NETCFG_POM_ND_GetRouterRedundancyProtocolIpNetToPhysicalEntry(NETCFG_OM_ND_RouterRedundancyEntry_T *entry)
{
    const UI32_T msg_size = NETCFG_OM_ND_GET_MSG_SIZE(arg_rp_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_ND_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_NDCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_ND_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETRPENTRY;

    memcpy(&(msg_p->data.arg_rp_entry), entry, sizeof(NETCFG_OM_ND_RouterRedundancyEntry_T));
    if (SYSFUN_SendRequestMsg(netcfg_om_nd_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(entry, &(msg_p->data.arg_rp_entry), sizeof(NETCFG_OM_ND_RouterRedundancyEntry_T));
    return msg_p->type.result_bool;
}

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
BOOL_T NETCFG_POM_ND_GetNextStaticEntry(UI8_T addr_type, NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry)
{
    const UI32_T msg_size = NETCFG_OM_ND_GET_MSG_SIZE(arg_nd_static_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_ND_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_NDCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_ND_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETNEXTSTATICENTRY;

    msg_p->data.arg_nd_static_entry.addr_type = addr_type;
    memcpy(&(msg_p->data.arg_nd_static_entry.entry), entry, sizeof(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T));
    if (SYSFUN_SendRequestMsg(netcfg_om_nd_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(entry, &(msg_p->data.arg_nd_static_entry.entry), sizeof(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T));
    return msg_p->type.result_bool;
}

#ifdef SYS_CPNT_IPV6

/* FUNCTION NAME : NETCFG_POM_ND_GetNdDADAttempts
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
BOOL_T NETCFG_POM_ND_GetNdDADAttempts(UI32_T vid_ifIndex, UI32_T *attempts)
{
    NETCFG_POM_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;

    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETNDDADATTEMPTS;
    msg_p->data.arg_ifindex_and_ui32.ifindex = vid_ifIndex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = *attempts;
    
    NETCFG_POM_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    *attempts = msg_p->data.arg_ifindex_and_ui32.ui32_v;
    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_POM_ND_GetRunningDADAttempts
 * PURPOSE:
 *    Get running DAD ( duplicate address detection) attempts 
 * INPUT:
 *    vid_ifindex.
 *
 * OUTPUT:
 *    attempts.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ND_GetRunningDADAttempts(UI32_T vid_ifindex, UI32_T* attempts)
{
    if ( TRUE == NETCFG_POM_ND_GetNdDADAttempts(vid_ifindex, attempts))
    {
        if (SYS_DFLT_ND_DUPLICATE_ADDRESS_DETECTION_ATTEMPTS != *attempts)
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}


/* FUNCTION NAME : NETCFG_POM_ND_GetNdNsInterval
 * PURPOSE:
 *    get  the interval between IPv6 neighbor solicitation retransmissions on an interface 
 * INPUT:
 *    vid_ifindex
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
BOOL_T NETCFG_POM_ND_GetNdNsInterval(UI32_T vid_ifindex, UI32_T *msec)
{
    NETCFG_POM_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;


    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETNDNSINTERVAL;
    msg_p->data.arg_ifindex_and_ui32.ifindex =vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = *msec;

    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    *msec = msg_p->data.arg_ifindex_and_ui32.ui32_v;
    return msg_p->type.result_bool;

}


/* FUNCTION NAME : NETCFG_POM_ND_GetRunningNdNsInterval
 * PURPOSE:
 *    get running config of the NS interval on an interface
 * INPUT:
 *    vid_ifIndex
 *
 * OUTPUT:
 *    msec.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ND_GetRunningNdNsInterval(UI32_T vid_ifindex, UI32_T* msec)
{
    if (NETCFG_POM_ND_GetNdNsInterval(vid_ifindex, msec))
    {
        if (*msec != SYS_DFLT_ND_NEIGHBOR_SOLICITATION_RETRANSMISSIONS_INTERVAL)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
}


/* FUNCTION NAME : NETCFG_POM_ND_GetNdHoplimit
 * PURPOSE:
 *    get  the maximum number of hops used in router advertisements 
 * INPUT:
 *    vid_ifindex
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
BOOL_T NETCFG_POM_ND_GetNdHoplimit(UI32_T vid_ifindex,UI32_T *hoplimit)
{
    NETCFG_POM_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;


    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETNDHOPLIMIT;
    msg_p->data.arg_ifindex_and_ui32.ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = *hoplimit;
    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    *hoplimit = msg_p->data.ui32_v;
    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_POM_ND_GetNdPrefix
 * PURPOSE:
 *    get  which IPv6 prefixes are included in IPv6 router advertisements
 * INPUT:
 *     L_INET_AddrIp_T: prefix
 *    UI32_T vid_ifindex ,  validLifetime, preferredLifetime
 *    BOOL_T: offLink. noAutoconfig
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
 BOOL_T NETCFG_POM_ND_GetNdPrefix(UI32_T vid_ifindex, L_INET_AddrIp_T *prefix, UI32_T *validLifetime, UI32_T *preferredLifetime,BOOL_T *enable_onlink,BOOL_T *enable_autoconf)
{
    NETCFG_POM_DECLARE_MSG_P(arg_nd_prefix)
    UI32_T result;


    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETNDPREFIX;
    msg_p->data.arg_nd_prefix.ifIndex = vid_ifindex;
    msg_p->data.arg_nd_prefix.prefix = *prefix;
    msg_p->data.arg_nd_prefix.vlifetime = 0;
    msg_p->data.arg_nd_prefix.plifetime = 0;
    msg_p->data.arg_nd_prefix.enable_onlink = 0;
    msg_p->data.arg_nd_prefix.enable_autoaddr = 0;
    
    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    *prefix = msg_p->data.arg_nd_prefix.prefix;
    *validLifetime = msg_p->data.arg_nd_prefix.vlifetime;
    *preferredLifetime = msg_p->data.arg_nd_prefix.plifetime;
    *enable_onlink = msg_p->data.arg_nd_prefix.enable_onlink;
    *enable_autoconf = msg_p->data.arg_nd_prefix.enable_autoaddr;
    
    return msg_p->type.result_bool;

}
BOOL_T NETCFG_POM_ND_GetNdDefaultPrefixConfig( UI32_T *validLifetime, UI32_T *preferredLifetime,BOOL_T *enable_onlink,BOOL_T *enable_autoconf)
{
  
    *validLifetime = SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_VALID_LIFETIME;
    *preferredLifetime = SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_PREFERRED_LIFETIME;
    *enable_onlink = (SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_FLAG_ON_LINK)?TRUE:FALSE;
    *enable_autoconf = (SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_FLAG_AUTO_ADDRESS)?TRUE:FALSE;

    return TRUE;
}
/* FUNCTION NAME : NETCFG_POM_ND_GetNdManagedConfigFlag
 * PURPOSE:
 *    get    the "managed address configuration" flag in IPv6 router advertisements
 * INPUT:
 *    vid_ifindex
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
BOOL_T NETCFG_POM_ND_GetNdManagedConfigFlag(UI32_T vid_ifindex, BOOL_T *enableFlag)
{
    NETCFG_POM_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;


    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETMANAGEDFLAG;
    msg_p->data.arg_ifindex_and_bool.ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_bool.bool_v = *enableFlag;

    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    *enableFlag = msg_p->data.arg_ifindex_and_bool.bool_v;
    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_POM_ND_GetNdOtherConfigFlag
 * PURPOSE:
 *    get    the "other stateful configuration" flag in IPv6 router advertisements
 * INPUT:
 *    vid_ifindex
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
BOOL_T NETCFG_POM_ND_GetNdOtherConfigFlag(UI32_T vid_ifindex, BOOL_T *enableFlag)
{
    NETCFG_POM_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;


    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETOTHERFLAG;
    msg_p->data.arg_ifindex_and_bool.ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_bool.bool_v = *enableFlag;

    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    *enableFlag = msg_p->data.arg_ifindex_and_bool.bool_v;
    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_POM_ND_GetNdReachableTime
 * PURPOSE:
 *    get    the amount of time that a remote IPv6 node is considered reachable  
 *                  after some reachability confirmation event has occurred
 * INPUT:
 *    vid_ifindex
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
BOOL_T NETCFG_POM_ND_GetNdReachableTime(UI32_T vid_ifindex,UI32_T *msec)
{
    NETCFG_POM_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;


    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETNDREACHABLETIME;
    msg_p->data.arg_ifindex_and_ui32.ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = *msec;

    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    *msec = msg_p->data.arg_ifindex_and_ui32.ui32_v;
    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_POM_ND_GetRunningNdReachableTime
 * PURPOSE:
 *    get running config of reachable time on an interface
 * INPUT:
 *    vid_ifIndex
 *
 * OUTPUT:
 *    msec.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ND_GetRunningNdReachableTime(UI32_T vid_ifindex, UI32_T* msec)
{
    if (NETCFG_POM_ND_GetNdReachableTime(vid_ifindex, msec))
    {
        if (*msec != SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_REACHABLE_TIME)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
}

/* FUNCTION NAME : NETCFG_POM_ND_GetNdRaSuppress
 * PURPOSE:
 *    get whether suppress  IPv6 router advertisement transmissions
 * INPUT:
 *    vid_ifindex
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
BOOL_T NETCFG_POM_ND_GetNdRaSuppress(UI32_T vid_ifindex, BOOL_T *enableSuppress)
{
    NETCFG_POM_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;


    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETRASUPPRESS;
    msg_p->data.arg_ifindex_and_bool.ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_bool.bool_v = *enableSuppress;

    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    *enableSuppress = msg_p->data.arg_ifindex_and_bool.bool_v;
    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_POM_ND_GetNdRaLifetime
 * PURPOSE:
 *    get the router lifetime value in IPv6 router advertisements 
 * INPUT:
 *    vid_ifindex
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
BOOL_T NETCFG_POM_ND_GetNdRaLifetime(UI32_T vid_ifindex, UI32_T *seconds)
{
    NETCFG_POM_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;


    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETRALIFETIME;
    msg_p->data.arg_ifindex_and_ui32.ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = *seconds;

    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    *seconds = msg_p->data.arg_ifindex_and_ui32.ui32_v;
    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_POM_ND_GetNdRaInterval
 * PURPOSE:
 *    get the interval between IPv6 router advertisement  transmissions
 * INPUT:
 *    vid_ifindex -- vlan ifindex
 *
 * OUTPUT:
 *    max_p  -- max ra interval
 *    min_p  -- min ra interval
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdRaInterval(UI32_T vid_ifindex, UI32_T *max_p, UI32_T *min_p)
{
    NETCFG_POM_DECLARE_MSG_P(arg_ifindex_and_ui32x2)
    UI32_T result;

    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETRAINTERVAL;
    msg_p->data.arg_ifindex_and_ui32x2.ifindex = vid_ifindex;

    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    *max_p = msg_p->data.arg_ifindex_and_ui32x2.ui32_1_v;
    *min_p = msg_p->data.arg_ifindex_and_ui32x2.ui32_2_v;

    return msg_p->type.result_bool;
}

/* FUNCTION NAME : NETCFG_POM_ND_GetNdRouterPreference
 * PURPOSE:
 *    get the the Preference flag in IPv6 router advertisements
 * INPUT:
 *    vid_ifindex
 *    prefer.
 *
 * OUTPUT:
 *    prefer.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdRouterPreference(UI32_T vid_ifindex, UI32_T  *prefer)
{
    NETCFG_POM_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;


    msg_p->type.cmd = NETCFG_OM_ND_IPC_GETRAROUTERPREFERENCE;
    msg_p->data.arg_ifindex_and_ui32.ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = *prefer;

    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    *prefer = msg_p->data.arg_ifindex_and_ui32.ui32_v;
    return msg_p->type.result_bool;

}

BOOL_T NETCFG_POM_ND_IsConfigFlagSet(UI32_T vid_ifindex, UI32_T flag, BOOL_T *is_set)
{
    NETCFG_POM_DECLARE_MSG_P(arg_ui32_ui32_bool)
    UI32_T result;


    msg_p->type.cmd = NETCFG_OM_ND_IPC_ISCONFIGFLAGSET;
    msg_p->data.arg_ui32_ui32_bool.ui32_v1 = vid_ifindex;
    msg_p->data.arg_ui32_ui32_bool.ui32_v2 = flag;

    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    *is_set = msg_p->data.arg_ui32_ui32_bool.bool_v;
    return msg_p->type.result_bool;

}
#endif /*SYS_CPNT_IPV6*/

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
/* FUNCTION NAME : NETCFG_OM_ND_RAGUARD_IsEnabled
 * PURPOSE:
 *    To check if RA Guard is enabled for specifed lport.
 * INPUT:
 *    lport    - which lport to check (1-based)
 *    pkt_type - which packet type received,
 *               NETCFG_TYPE_RG_PKT_MAX to skip statistics
 *               (NETCFG_TYPE_RG_PKT_RA/NETCFG_TYPE_RG_PKT_RR)
 * OUTPUT:
 *
 * RETURN:
 *    TRUE  --  Success
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_RAGUARD_IsEnabled(
    UI32_T  lport,
    UI32_T  pkt_type)
{
    NETCFG_POM_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;

    msg_p->type.cmd = NETCFG_OM_ND_IPC_RA_GUARD_ISENABLED;
    msg_p->data.arg_ifindex_and_ui32.ifindex = lport;
    msg_p->data.arg_ifindex_and_ui32.ui32_v  = pkt_type;

    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* FUNCTION NAME : NETCFG_POM_ND_RAGUARD_IsAnyPortEnabled
 * PURPOSE:
 *    To check if RA Guard is enabled for any port.
 * INPUT:
 *    None.
 * OUTPUT:
 *    None.
 * RETURN:
 *    TRUE  --  Success
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_RAGUARD_IsAnyPortEnabled(void)
{
    NETCFG_POM_DECLARE_MSG_P(arg_empty)
    UI32_T result;

    msg_p->type.cmd = NETCFG_OM_ND_IPC_RA_GUARD_ISANYPORTENABLED;

    NETCFG_POM_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

static L_INET_AddrIp_T NETCFG_POM_ND_ComposeInetAddr(UI16_T type,UI8_T* addrp,UI16_T prefixLen)
{
    L_INET_AddrIp_T addr;
    /*2009-11-06 Jimi, must initial the unused field when convert*/
    memset(&addr, 0, sizeof(L_INET_AddrIp_T));
    if(L_INET_ADDR_TYPE_IPV4 == type)
    {
        addr.type = type;
        addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    }
    else if(type == L_INET_ADDR_TYPE_IPV6)
    {
        addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
        if(L_INET_ADDR_IS_IPV6_LINK_LOCAL(addrp))
            addr.type = L_INET_ADDR_TYPE_IPV6Z;
        else
            addr.type = type;

    }
    else
    {   
        printf ("Oops! something wrong!\r\n");
    }
    
    memcpy(addr.addr,addrp,addr.addrlen);
    addr.preflen = prefixLen;
    return addr;
}
void NETCFG_POM_ND_ConvertIpNetToMediaToIpNetToPhysical (NETCFG_TYPE_IpNetToPhysicalEntry_T* output ,NETCFG_TYPE_IpNetToMediaEntry_T* input)
{
    memset(output,0,sizeof(NETCFG_TYPE_IpNetToPhysicalEntry_T));
    output->ip_net_to_physical_if_index = input->ip_net_to_media_if_index;
    output->ip_net_to_physical_phys_address = input->ip_net_to_media_phys_address;
    output->ip_net_to_physical_net_address =NETCFG_POM_ND_ComposeInetAddr(L_INET_ADDR_TYPE_IPV4,(UI8_T*) &input->ip_net_to_media_net_address, 0);
    output->ip_net_to_physical_type = input->ip_net_to_media_type;
}
void NETCFG_POM_ND_ConvertIpNetToPhysicalToIpNetToMedia (NETCFG_TYPE_IpNetToMediaEntry_T* output ,   NETCFG_TYPE_IpNetToPhysicalEntry_T* input)
{
    memset(output,0,sizeof(NETCFG_TYPE_IpNetToMediaEntry_T));
    output->ip_net_to_media_if_index = input->ip_net_to_physical_if_index;
    output->ip_net_to_media_phys_address = input->ip_net_to_physical_phys_address;
    memcpy(&output->ip_net_to_media_net_address,input->ip_net_to_physical_net_address.addr ,SYS_ADPT_IPV4_ADDR_LEN);
    output->ip_net_to_media_type = input->ip_net_to_physical_type;
}

void NETCFG_POM_ND_ConvertIpv6NetToMediaToIpNetToPhysical (NETCFG_TYPE_IpNetToPhysicalEntry_T* output ,NETCFG_TYPE_Ipv6NetToMediaEntry_T* input)
{
    memset(output,0,sizeof(NETCFG_TYPE_IpNetToPhysicalEntry_T));
    output->ip_net_to_physical_if_index = input->ip_net_to_media_if_index;
    output->ip_net_to_physical_phys_address = input->ip_net_to_media_phys_address;
    output->ip_net_to_physical_net_address =NETCFG_POM_ND_ComposeInetAddr(L_INET_ADDR_TYPE_IPV6, input->ip_net_to_media_net_address, 0);
    output->ip_net_to_physical_type = input->ip_net_to_media_type;
}
void NETCFG_POM_ND_ConvertIpNetToPhysicalToIpv6NetToMedia (NETCFG_TYPE_Ipv6NetToMediaEntry_T* output ,   NETCFG_TYPE_IpNetToPhysicalEntry_T* input)
{
    memset(output,0,sizeof(NETCFG_TYPE_Ipv6NetToMediaEntry_T));
    output->ip_net_to_media_if_index = input->ip_net_to_physical_if_index;
    output->ip_net_to_media_phys_address = input->ip_net_to_physical_phys_address;
    memcpy(output->ip_net_to_media_net_address,input->ip_net_to_physical_net_address.addr,SYS_ADPT_IPV6_ADDR_LEN);
    output->ip_net_to_media_type= input->ip_net_to_physical_type;
}



