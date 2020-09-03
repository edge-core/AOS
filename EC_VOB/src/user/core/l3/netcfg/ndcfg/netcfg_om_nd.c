/* Module Name:NETCFG_OM_ND.C
 * Purpose: To store configuration static arp and arp aging timeout.
 *
 * Notes:
 *      1.
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *    2008/01/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008.
 */

#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "l_mm.h"
#include "l_pt.h"
#include "l_sort_lst.h"
#include "sysfun.h"
#include "netcfg_om_nd.h"
//#include "netcfg_om_nd_private.h"
#include "netcfg_type.h"
#include "vlan_lib.h"
#include "netcfg_pom_nd.h"  /*for convert funtion*/
#include "ip_lib.h"
#include "backdoor_mgr.h"

typedef    struct NETCFG_OM_ND_LCB_S
{
    UI32_T                        sem_id;
    SYS_TYPE_Stacking_Mode_T    stacking_mode;
} NETCFG_OM_ND_LCB_T;

static int NETCFG_OM_ND_Compare(void *elm1,void *elm2);
static int NETCFG_OM_ND_RouterRedundancyEntryCompare(void *elm1,void *elm2);
static int NETCFG_OM_ND_CompareV6(void *elm1,void *elm2);
static int NETCFG_OM_ND_RAPrefixCompare(void *elm1,void *elm2);
static NETCFG_OM_ND_CONFIG_T* NETCFG_OM_ND_GetNdconfigByIfindex(UI32_T ifindex);


static NETCFG_OM_ND_LCB_T neighbor_lcb;   /* semophore variable */
#define NETCFG_OM_NDV4 0 /*neighbors[NETCFG_OM_NDV4]*/
#define NETCFG_OM_NDV6 1 /*neighbors[NETCFG_OM_NDV6]*/
static L_SORT_LST_List_T neighbors[2];/* a list to store  neighbor entry*/
static L_SORT_LST_List_T rp_entries; /* a list to store router redundancy entry */
static UI32_T    ipv4_arp_timeout;
//static UI8_T   ra_hop_limit;

/* Jimi, seperate normal interface and tunnel interface */
/*static NETCFG_OM_ND_CONFIG_T nd_cfg[SYS_ADPT_MAX_VLAN_ID+SYS_ADPT_MAX_TUNNEL_ID];*/
static NETCFG_OM_ND_CONFIG_T nd_cfg[SYS_ADPT_MAX_VLAN_ID];
#if(SYS_CPNT_IP_TUNNEL == TRUE)
static NETCFG_OM_ND_CONFIG_T tunnel_nd_cfg[SYS_ADPT_MAX_TUNNEL_ID];
#endif



/* debug function*/
#define DEBUG_FLAG_BIT_DEBUG 0x01
#define DEBUG_FLAG_BIT_INFO  0x02
#define DEBUG_FLAG_BIT_NOTE  0x04
/***************************************************************/
static UI32_T DEBUG_FLAG = 0; //DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO|DEBUG_FLAG_BIT_NOTE;
/***************************************************************/
#define DBGprintf(format,args...) ((DEBUG_FLAG_BIT_DEBUG & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define INFOprintf(format,args...) ((DEBUG_FLAG_BIT_INFO & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define NOTEprintf(format,args...) ((DEBUG_FLAG_BIT_NOTE & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)

/*Simon's dump memory  function*/
#define DUMP_MEMORY_BYTES_PER_LINE 32
#define DUMP_MEMORY(mptr,size) do{\
    int i;\
    unsigned char * ptr=(unsigned char *)mptr;\
    for(i=0;i<size;i++){\
        if(i%DUMP_MEMORY_BYTES_PER_LINE ==0){\
            if(i>0)printf("\r\n");\
            printf("%0.4xH\t",i);\
        }\
        printf("%0.2x", *ptr++);\
    }\
    printf("\r\n");\
}while(0)
/*END  debug function*/

static char  ip_string_buffer[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) = (V) | (F)
#define UNSET_FLAG(V,F)      (V) = (V) & ~(F)
#define FLAG_ISSET(V,F)      (((V) & (F)) == (F))

static NETCFG_OM_ND_CONFIG_T* NETCFG_OM_ND_GetNdconfigByIfindex(UI32_T ifindex)
{
    UI32_T vid=0;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
{
    UI32_T tunnel_id = 0;
    if(IS_TUNNEL_IFINDEX(ifindex))
    {
        if(FALSE == IP_LIB_ConvertTunnelIdFromIfindex(ifindex, &tunnel_id))
        {
            DBGprintf("invalid tunnel ifindex=%ld", (long)vid);
            return NULL;
        }
        return &tunnel_nd_cfg[tunnel_id-1];
    }
}
#endif
    if( VLAN_OM_ConvertFromIfindex(ifindex, &vid))
    {
        if( vid > 0 && vid <= (sizeof(nd_cfg)/sizeof(nd_cfg[0])))
            return &nd_cfg[vid-1];
        else
        {
            DBGprintf("invalid ifindex=%ld", (long)vid);
            return NULL;
        }
    }
    //else if(TUNNEL_OM_ConvertFromIfindex(ifindex, &vid))
    //    return &nd_cfg[SYS_ADPT_MAX_VLAN_ID+vid-1];
    else
        return NULL;
}


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
BOOL_T NETCFG_OM_ND_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    NETCFG_OM_ND_IPCMsg_T *netcfg_om_arp_msg_p;


    if(ipcmsg_p==NULL)
    {
        DBGprintf("null input");
        return FALSE;
    }
    netcfg_om_arp_msg_p= (NETCFG_OM_ND_IPCMsg_T*)ipcmsg_p->msg_buf;
/*
     ,
     ,
    ,
    ,
    ,
    ,
    ,
    ,
    ,
    ,
*/
    switch(netcfg_om_arp_msg_p->type.cmd)
    {
        case     NETCFG_OM_ND_IPC_GETTIMEOUT:
            netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetTimeout(&(netcfg_om_arp_msg_p->data.ui32_v));
            ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(ui32_v);
            break;
        case NETCFG_OM_ND_IPC_GETRPENTRY:
            netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetRouterRedundancyProtocolIpNetToPhysicalEntry(&(netcfg_om_arp_msg_p->data.arg_rp_entry));
            ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_rp_entry);
            break;
        case NETCFG_OM_ND_IPC_GETNEXTSTATICENTRY:
            netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetNextStaticEntry(netcfg_om_arp_msg_p->data.arg_nd_static_entry.addr_type,
                                                                                    &netcfg_om_arp_msg_p->data.arg_nd_static_entry.entry);
            ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_nd_static_entry);
            break;
#if (SYS_CPNT_IPV6 == TRUE)
        case NETCFG_OM_ND_IPC_GETNDDADATTEMPTS:
                netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetNdDADAttempts(netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ifindex,&netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ui32_v);
                ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_ifindex_and_ui32);
            break;
        case NETCFG_OM_ND_IPC_GETNDNSINTERVAL:
                netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetNdNsInterval(netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ifindex, &netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ui32_v);
                ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_ifindex_and_ui32);
            break;
        case NETCFG_OM_ND_IPC_GETNDHOPLIMIT:
                netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetNdHoplimit(netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ifindex, &netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ui32_v);
                ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(ui32_v);
            break;
        case NETCFG_OM_ND_IPC_GETNDPREFIX:
                netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetNdPrefix(
                                                            netcfg_om_arp_msg_p->data.arg_nd_prefix.ifIndex,
                                                            &netcfg_om_arp_msg_p->data.arg_nd_prefix.prefix,
                                                            &netcfg_om_arp_msg_p->data.arg_nd_prefix.vlifetime,
                                                            &netcfg_om_arp_msg_p->data.arg_nd_prefix.plifetime,
                                                            &netcfg_om_arp_msg_p->data.arg_nd_prefix.enable_onlink,
                                                            &netcfg_om_arp_msg_p->data.arg_nd_prefix.enable_autoaddr);

                ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_nd_prefix);
            break;
        case NETCFG_OM_ND_IPC_GETMANAGEDFLAG:
                netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetNdManagedConfigFlag(netcfg_om_arp_msg_p->data.arg_ifindex_and_bool.ifindex,&netcfg_om_arp_msg_p->data.arg_ifindex_and_bool.bool_v);
                ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_ifindex_and_bool);
            break;
        case NETCFG_OM_ND_IPC_GETOTHERFLAG:
                netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetNdOtherConfigFlag(netcfg_om_arp_msg_p->data.arg_ifindex_and_bool.ifindex,&netcfg_om_arp_msg_p->data.arg_ifindex_and_bool.bool_v);
                ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_ifindex_and_bool);
            break;
        case NETCFG_OM_ND_IPC_GETNDREACHABLETIME:
                netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetNdReachableTime(netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ifindex, &netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ui32_v);
                ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_ifindex_and_ui32);
            break;
        case NETCFG_OM_ND_IPC_GETRASUPPRESS:
                netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetNdRaSuppress(netcfg_om_arp_msg_p->data.arg_ifindex_and_bool.ifindex, &netcfg_om_arp_msg_p->data.arg_ifindex_and_bool.bool_v);
                ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_ifindex_and_bool);
            break;
        case NETCFG_OM_ND_IPC_GETRAROUTERPREFERENCE:
                netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetNdRouterPreference(netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ifindex, &netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ui32_v);
                ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_ifindex_and_bool);
            break;
        case NETCFG_OM_ND_IPC_GETRALIFETIME:
                netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetNdRaLifetime(netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ifindex, &netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ui32_v);
                ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_ifindex_and_ui32);
            break;
        case NETCFG_OM_ND_IPC_GETRAINTERVAL:
                netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_GetNdRaInterval(netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32x2.ifindex,
                                                            &netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32x2.ui32_1_v,
                                                            &netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32x2.ui32_2_v);
                ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_ifindex_and_ui32x2);
            break;

        case NETCFG_OM_ND_IPC_ISCONFIGFLAGSET:
                netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_IsConfigFlagSet(netcfg_om_arp_msg_p->data.arg_ui32_ui32_bool.ui32_v1, netcfg_om_arp_msg_p->data.arg_ui32_ui32_bool.ui32_v2, &(netcfg_om_arp_msg_p->data.arg_ui32_ui32_bool.bool_v));
                ipcmsg_p->msg_size=NETCFG_OM_ND_GET_MSG_SIZE(arg_ui32_ui32_bool);
            break;

#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
        case NETCFG_OM_ND_IPC_RA_GUARD_ISENABLED:
            netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_RAGUARD_IsEnabled(
                netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ifindex,
                netcfg_om_arp_msg_p->data.arg_ifindex_and_ui32.ui32_v);
            ipcmsg_p->msg_size=NETCFG_OM_ND_MSGBUF_TYPE_SIZE;
            break;

        case NETCFG_OM_ND_IPC_RA_GUARD_ISANYPORTENABLED:
            netcfg_om_arp_msg_p->type.result_bool = NETCFG_OM_ND_RAGUARD_IsAnyPortEnabled();
            ipcmsg_p->msg_size=NETCFG_OM_ND_MSGBUF_TYPE_SIZE;
            break;
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            netcfg_om_arp_msg_p->type.result_bool = FALSE;
            ipcmsg_p->msg_size = NETCFG_OM_ND_MSGBUF_TYPE_SIZE;
            DBGprintf("Unknow message!");
            return FALSE;
    }
    return TRUE;
}

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
void NETCFG_OM_ND_Init(void)
{
    UI32_T i,vid_ifindex;
    ipv4_arp_timeout = SYS_DFLT_IP_NET_TO_MEDIA_ENTRY_TIMEOUT;

#if (SYS_CPNT_IPV6 == TRUE)
    for(i=1;i< SYS_ADPT_MAX_VLAN_ID ;i++)
    {
        VLAN_OM_ConvertToIfindex(i, &vid_ifindex);
        NETCFG_OM_ND_InitNdConfig(vid_ifindex);
    }
#if (SYS_CPNT_IP_TUNNEL == TRUE)
{
    UI32_T tunnel_ifindex = 0;
    for(i=1;i<= SYS_ADPT_MAX_TUNNEL_ID;i++)
    {
        IP_LIB_ConvertTunnelIdToIfindex(i, &tunnel_ifindex);
        NETCFG_OM_ND_InitNdConfig(tunnel_ifindex);
    }
}
#endif
#endif

    if(L_SORT_LST_Create(&neighbors[NETCFG_OM_NDV4],
                      SYS_ADPT_MAX_NBR_OF_STATIC_NEIGHBOR_CACHE_ENTRY,
                      sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T),
                      NETCFG_OM_ND_Compare)==FALSE)
    {
        /* DEBUG */
        DBGprintf("NETCFG_OM_ND_Init failed!");
        SYSFUN_LogDebugMsg("NETCFG_OM_ND_Init : Can't create neighbor[0] List.\n");
    }

    if (L_SORT_LST_Create(&rp_entries,
                            SYS_ADPT_MAX_NUMBER_OF_VIRTUAL_ROUTER,
                            sizeof(NETCFG_OM_ND_RouterRedundancyEntry_T),
                            NETCFG_OM_ND_RouterRedundancyEntryCompare) == FALSE)
    {
        DBGprintf("NETCFG_OM_ND_Init failed!");
        SYSFUN_LogDebugMsg("NETCFG_OM_ND_Init : Can't create rp_entries List.\n");
    }

#if (SYS_CPNT_IPV6 == TRUE)
    if(L_SORT_LST_Create(&neighbors[NETCFG_OM_NDV6],
                      SYS_ADPT_MAX_NBR_OF_STATIC_NEIGHBOR_CACHE_ENTRY,
                      sizeof(NETCFG_TYPE_StaticIpv6NetToMediaEntry_T),
                      NETCFG_OM_ND_CompareV6)==FALSE)
    {
        /* DEBUG */
        DBGprintf("NETCFG_OM_ND_Init failed!");
        SYSFUN_LogDebugMsg("NETCFG_OM_ND_Init : Can't create neighbor [1]List.\n");
    }
#endif

    memset(&neighbor_lcb,0,sizeof(neighbor_lcb));

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NETCFG_OM, &neighbor_lcb.sem_id) != SYSFUN_OK)
    {
        DBG_PrintText("NETCFG_OM_ND_Init : Can't create semaphore\n");
    }
}

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
BOOL_T NETCFG_OM_ND_AddStaticEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T     original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    if(entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV4 ||
       entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV4Z)
    {
        NETCFG_TYPE_StaticIpNetToMediaEntry_T staticv4;
        NETCFG_POM_ND_ConvertIpNetToPhysicalToIpNetToMedia(&staticv4.ip_net_to_media_entry,&entry->ip_net_to_physical_entry);
        staticv4.status = entry->status;
        result = L_SORT_LST_Set(&neighbors[NETCFG_OM_NDV4], &staticv4);
    }
    else if (entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV6 ||
             entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV6Z)
    {
        NETCFG_TYPE_StaticIpv6NetToMediaEntry_T  staticv6;
        NETCFG_POM_ND_ConvertIpNetToPhysicalToIpv6NetToMedia(&staticv6.ip_net_to_media_entry,&entry->ip_net_to_physical_entry);
        staticv6.status = entry->status;
        result = L_SORT_LST_Set(&neighbors[NETCFG_OM_NDV6], &staticv6);
        {
            if(FALSE == L_INET_Ntop(L_INET_AF_INET6, (UI8_T *)&staticv6.ip_net_to_media_entry.ip_net_to_media_net_address, ip_string_buffer, sizeof(ip_string_buffer)))
                sprintf(ip_string_buffer,"UNKNOWNADDR");
            INFOprintf("insert to v(%ld)%s type %d", (long)staticv6.ip_net_to_media_entry.ip_net_to_media_if_index,ip_string_buffer,staticv6.ip_net_to_media_entry.ip_net_to_media_type);
            INFOprintf("insert to %x-%x-%x-%x-%x-%x",L_INET_EXPAND_MAC(staticv6.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string));
        }
    }
    else
    {
        DBGprintf("unknown addr type!");
        result = FALSE;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return  result;
}

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
BOOL_T    NETCFG_OM_ND_DeleteStaticEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T     original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    if(entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV4 ||
        entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV4Z)
    {
        NETCFG_TYPE_StaticIpNetToMediaEntry_T staticv4;
        NETCFG_POM_ND_ConvertIpNetToPhysicalToIpNetToMedia(&staticv4.ip_net_to_media_entry,&entry->ip_net_to_physical_entry);
        staticv4.status = entry->status;
        result = L_SORT_LST_Delete(&neighbors[NETCFG_OM_NDV4], &staticv4);
    }
    else if (entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV6 ||
                entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV6Z)
    {
        NETCFG_TYPE_StaticIpv6NetToMediaEntry_T  staticv6;
        NETCFG_POM_ND_ConvertIpNetToPhysicalToIpv6NetToMedia(&staticv6.ip_net_to_media_entry,&entry->ip_net_to_physical_entry);
        staticv6.status = entry->status;
        result = L_SORT_LST_Delete(&neighbors[NETCFG_OM_NDV6], &staticv6);
    }
    else
    {
        DBGprintf("Unknow addr type!");
        result = FALSE;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return  result;
}

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
BOOL_T    NETCFG_OM_ND_UpdateStaticEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T     original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    if(entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type== L_INET_ADDR_TYPE_IPV4 ||
        entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV4Z)
    {
        NETCFG_TYPE_StaticIpNetToMediaEntry_T *mem_entry, local_entry;
        NETCFG_POM_ND_ConvertIpNetToPhysicalToIpNetToMedia(&local_entry.ip_net_to_media_entry,&entry->ip_net_to_physical_entry);
        local_entry.status =entry->status;
        mem_entry = L_SORT_LST_GetPtr(&neighbors[NETCFG_OM_NDV4], &local_entry);

        if(mem_entry != NULL)
        {
            memcpy(mem_entry, &local_entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
            result = TRUE;
        }
        else
        {
            DBGprintf("Fail to get from neighbors[ipv4]");
            result = FALSE;
        }
    }
    else if (entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV6 ||
                entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV6Z)
    {
        NETCFG_TYPE_StaticIpv6NetToMediaEntry_T *mem_entry,local_entry;
        NETCFG_POM_ND_ConvertIpNetToPhysicalToIpv6NetToMedia(&local_entry.ip_net_to_media_entry,&entry->ip_net_to_physical_entry);
        local_entry.status =entry->status;
        mem_entry = L_SORT_LST_GetPtr(&neighbors[NETCFG_OM_NDV6], &local_entry);
        if(mem_entry != NULL)
        {
            memcpy(mem_entry, &local_entry, sizeof(NETCFG_TYPE_StaticIpv6NetToMediaEntry_T));
            result = TRUE;
        }
        else
        {
            DBGprintf("Fail to get from neighbors[ipv6]");
            result = FALSE;
        }
    }
    else
    {
        DBGprintf("unknown addr type!");
        result = FALSE;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return  result;
}

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
BOOL_T NETCFG_OM_ND_GetStaticEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry)
{
    BOOL_T  result = FALSE;
    UI32_T  original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    if(entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV4 ||
       entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV4Z)
    {
        NETCFG_TYPE_StaticIpNetToMediaEntry_T staticv4;
        NETCFG_POM_ND_ConvertIpNetToPhysicalToIpNetToMedia(&staticv4.ip_net_to_media_entry, &entry->ip_net_to_physical_entry);
        staticv4.status = entry->status;
        result = L_SORT_LST_Get(&neighbors[NETCFG_OM_NDV4], &staticv4);
        NETCFG_POM_ND_ConvertIpNetToMediaToIpNetToPhysical(&entry->ip_net_to_physical_entry,&staticv4.ip_net_to_media_entry);
        entry->status = staticv4.status;
    }
    else if (entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV6 ||
             entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV6Z)
    {
        NETCFG_TYPE_StaticIpv6NetToMediaEntry_T staticv6;
        NETCFG_POM_ND_ConvertIpNetToPhysicalToIpv6NetToMedia(&staticv6.ip_net_to_media_entry, &entry->ip_net_to_physical_entry);
        staticv6.status = entry->status;
        result = L_SORT_LST_Get(&neighbors[NETCFG_OM_NDV6], &staticv6);
        NETCFG_POM_ND_ConvertIpv6NetToMediaToIpNetToPhysical(&entry->ip_net_to_physical_entry,&staticv6.ip_net_to_media_entry);
        entry->status = staticv6.status;
    }
    else{
        DBGprintf("Unknown addr type!");
        result = FALSE;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return  result;
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
BOOL_T NETCFG_OM_ND_GetNextStaticEntry(UI8_T addr_type, NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry)
{
    UI32_T          original_priority;
    L_INET_AddrIp_T null_addr;
    BOOL_T          result = FALSE;

    memset(&null_addr,0,sizeof(null_addr));
    BOOL_T (*lstGetFun)(L_SORT_LST_List_T *list, void *element) = NULL;

    if (entry == NULL)
    {
        DBGprintf("null pointer!");
        return FALSE;
    }

    if (0 == entry->ip_net_to_physical_entry.ip_net_to_physical_if_index&&
        0 == memcmp(&null_addr,&entry->ip_net_to_physical_entry.ip_net_to_physical_net_address,sizeof(null_addr))
       )
    {
        lstGetFun = L_SORT_LST_Get_1st;
    }
    else
    {
        lstGetFun = L_SORT_LST_Get_Next;
    }

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    if(addr_type == L_INET_ADDR_TYPE_IPV4 || addr_type == L_INET_ADDR_TYPE_IPV4Z)
    {
        NETCFG_TYPE_StaticIpNetToMediaEntry_T staticv4;
        NETCFG_POM_ND_ConvertIpNetToPhysicalToIpNetToMedia(&staticv4.ip_net_to_media_entry, &entry->ip_net_to_physical_entry);
        staticv4.status=entry->status;
        result = lstGetFun( &neighbors[NETCFG_OM_NDV4], &staticv4);
        NETCFG_POM_ND_ConvertIpNetToMediaToIpNetToPhysical(&entry->ip_net_to_physical_entry,&staticv4.ip_net_to_media_entry);
        entry->status = staticv4.status;
    }
    else if (addr_type == L_INET_ADDR_TYPE_IPV6 || addr_type == L_INET_ADDR_TYPE_IPV6Z)
    {
        NETCFG_TYPE_StaticIpv6NetToMediaEntry_T staticv6;
        NETCFG_POM_ND_ConvertIpNetToPhysicalToIpv6NetToMedia(&staticv6.ip_net_to_media_entry, &entry->ip_net_to_physical_entry);
        staticv6.status=entry->status;
        result = lstGetFun( &neighbors[NETCFG_OM_NDV6], &staticv6);
        NETCFG_POM_ND_ConvertIpv6NetToMediaToIpNetToPhysical(&entry->ip_net_to_physical_entry,&staticv6.ip_net_to_media_entry);
        entry->status = staticv6.status;
    }
    else
    {
        DBGprintf("Unknown addr type!");
        result = FALSE;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);
    return  result;
}

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
void NETCFG_OM_ND_DeleteAllStaticEntry(void)
{
    UI32_T   original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    L_SORT_LST_Delete_All(&neighbors[NETCFG_OM_NDV4]);
    L_SORT_LST_Delete_All(&neighbors[NETCFG_OM_NDV6]);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);
}

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
BOOL_T NETCFG_OM_ND_GetTimeout(UI32_T *age_time)
{
    UI32_T   original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    *age_time = ipv4_arp_timeout;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}


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
BOOL_T NETCFG_OM_ND_SetTimeout(UI32_T age_time)
{
    UI32_T     original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    ipv4_arp_timeout = age_time;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}

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
BOOL_T NETCFG_OM_ND_AddRouterRedundancyProtocolIpNetToPhysicalEntry(NETCFG_OM_ND_RouterRedundancyEntry_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T     original_priority;

    if (NULL == entry)
        return FALSE;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    result = L_SORT_LST_Set(&rp_entries, entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return result;
}

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
BOOL_T NETCFG_OM_ND_DeleteRouterRedundancyProtocolIpNetToPhysicalEntry(NETCFG_OM_ND_RouterRedundancyEntry_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T     original_priority;

    if (NULL == entry)
        return FALSE;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    result = L_SORT_LST_Delete(&rp_entries, entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return result;
}

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
BOOL_T NETCFG_OM_ND_GetRouterRedundancyProtocolIpNetToPhysicalEntry(NETCFG_OM_ND_RouterRedundancyEntry_T *entry)
{
    BOOL_T result = FALSE;
    UI32_T     original_priority;

    if (NULL == entry)
        return FALSE;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    result = L_SORT_LST_Get(&rp_entries, entry);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return result;
}

static int NETCFG_OM_ND_CompareIpAddr (UI8_T* addr1,UI8_T* addr2,int addrlen)
{
    int i;
    for(i=0;i<addrlen;i++)
    {
        if(addr1[i] > addr2[i])
            return 1;
        else if(addr1[i] < addr2[i])
            return -1;
    }
    return 0;
}

/* FUNCTION NAME : NETCFG_OM_ND_Compare
 * PURPOSE:
 *      Compare function of Sort-List.
 *
 * INPUT:
 *        elm1
 *          elm2
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      =0 : equal
 *      >0 : elm1 > elm2
 *      <0 : elm1 < elm2
 *
 * NOTES:  Key:ip_net_to_media_if_index and ip_net_to_media_net_address.
 */
 static int NETCFG_OM_ND_Compare(void *elm1,void *elm2)
 {
    NETCFG_TYPE_StaticIpNetToMediaEntry_T *element1, *element2;

    element1 = (NETCFG_TYPE_StaticIpNetToMediaEntry_T *)elm1;
    element2 = (NETCFG_TYPE_StaticIpNetToMediaEntry_T *)elm2;

#if 0
    if(element1->ip_net_to_media_entry.ip_net_to_media_if_index !=
       element2->ip_net_to_media_entry.ip_net_to_media_if_index)
    {
        if (element1->ip_net_to_media_entry.ip_net_to_media_if_index >
            element2->ip_net_to_media_entry.ip_net_to_media_if_index)
        {
            return (1);
        }
        else if (element1->ip_net_to_media_entry.ip_net_to_media_if_index <
                 element2->ip_net_to_media_entry.ip_net_to_media_if_index)
        {
            return (-1);
        }
    }
    else
    {
#endif
        if (element1->ip_net_to_media_entry.ip_net_to_media_net_address >
            element2->ip_net_to_media_entry.ip_net_to_media_net_address)
        {
            return (1);
        }
        else if (element1->ip_net_to_media_entry.ip_net_to_media_net_address <
                 element2->ip_net_to_media_entry.ip_net_to_media_net_address)
        {
            return (-1);
        }
        else
        {
            return (0);
        }
#if 0
    }
#endif
    return (0);
}

/* FUNCTION NAME : NETCFG_OM_ND_RouterRedundancyEntryCompare
 * PURPOSE:
 *      Compare function of Sort-List.
 *
 * INPUT:
 *        elm1
 *        elm2
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      =0 : equal
 *      >0 : elm1 > elm2
 *      <0 : elm1 < elm2
 *
 * NOTES:  Key:ip_addr.
 */
static int NETCFG_OM_ND_RouterRedundancyEntryCompare(void *elm1,void *elm2)
{
    NETCFG_OM_ND_RouterRedundancyEntry_T *element1, *element2;

    element1 = elm1;
    element2 = elm2;

    return memcmp(element1->ip_addr.addr, element2->ip_addr.addr,
                        SYS_ADPT_IPV4_ADDR_LEN);
}

#if (SYS_CPNT_IPV6 == TRUE)
static int NETCFG_OM_ND_CompareV6(void *elm1,void *elm2)
{
    NETCFG_TYPE_StaticIpv6NetToMediaEntry_T *element1 = (NETCFG_TYPE_StaticIpv6NetToMediaEntry_T*)elm1;
    NETCFG_TYPE_StaticIpv6NetToMediaEntry_T *element2 = (NETCFG_TYPE_StaticIpv6NetToMediaEntry_T*)elm2;

#if 0
    int ifIndexDiff;

    ifIndexDiff = element1->ip_net_to_media_entry.ip_net_to_media_if_index-
        element2->ip_net_to_media_entry.ip_net_to_media_if_index;
    if(ifIndexDiff>0)
        return 1;
    else if(ifIndexDiff<0)
        return -1;
    else
    {
#endif
        return NETCFG_OM_ND_CompareIpAddr(element1->ip_net_to_media_entry.ip_net_to_media_net_address,
                                                                            element2->ip_net_to_media_entry.ip_net_to_media_net_address,
                                                                            SYS_ADPT_IPV6_ADDR_LEN);
#if 0
    }
#endif
}   /* end of NETCFG_OM_ND_Compare  */

static int NETCFG_OM_ND_RAPrefixCompare(void *elm1,void *elm2)
 {
    NETCFG_OM_ND_PREFIX_T element1, element2;
    element1 = *(NETCFG_OM_ND_PREFIX_T *)elm1;
    element2 = *(NETCFG_OM_ND_PREFIX_T *)elm2;


    if(element1.ifIndex!= element2.ifIndex)
    {
        return (element1.ifIndex - element2.ifIndex);
    }
    if( element1.prefix.type==L_INET_ADDR_TYPE_IPV4Z)
        element1.prefix.type = L_INET_ADDR_TYPE_IPV4;
    if( element2.prefix.type==L_INET_ADDR_TYPE_IPV4Z)
        element2.prefix.type = L_INET_ADDR_TYPE_IPV4;
    if( element1.prefix.type==L_INET_ADDR_TYPE_IPV6Z)
        element1.prefix.type = L_INET_ADDR_TYPE_IPV6;
    if( element2.prefix.type==L_INET_ADDR_TYPE_IPV6Z)
        element2.prefix.type = L_INET_ADDR_TYPE_IPV6;


    if (element1.prefix.type!=element2.prefix.type)
    {
        if(element1.prefix.type==L_INET_ADDR_TYPE_IPV4)
            return 1;
        else
            return -1;
    }

    if(element1.prefix.preflen!=element2.prefix.preflen)
        return (element1.prefix.preflen-element2.prefix.preflen);

    if(element1.prefix.type == L_INET_ADDR_TYPE_IPV4)
        return memcmp(element1.prefix.addr,element2.prefix.addr,SYS_ADPT_IPV4_ADDR_LEN);
    else
        return  memcmp(element1.prefix.addr,element2.prefix.addr,SYS_ADPT_IPV6_ADDR_LEN);

    return (0);
}
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
BOOL_T NETCFG_OM_ND_SetNdDADAttempts(UI32_T vid_ifIndex,UI32_T  attempts)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    ndcfg->dad_attempts = attempts;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}


BOOL_T NETCFG_OM_ND_GetNdDADAttempts(UI32_T vid_ifIndex,UI32_T *attempts)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    *attempts = ndcfg->dad_attempts;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}

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
BOOL_T NETCFG_OM_ND_SetNdNsInterval(UI32_T vid_ifIndex, UI32_T msec)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    ndcfg->ns_interval = msec;
    SET_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_NS_INTERVAL);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_UnsetNdNsInterval(UI32_T vid_ifIndex)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    ndcfg->ns_interval = SYS_DFLT_ND_NEIGHBOR_SOLICITATION_RETRANSMISSIONS_INTERVAL;
    UNSET_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_NS_INTERVAL);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_GetNdNsInterval(UI32_T vid_ifIndex, UI32_T *msec)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    *msec = ndcfg->ns_interval ;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_IsSetNdNsInterval(UI32_T vid_ifIndex)
{
    UI32_T     original_priority;
    BOOL_T result;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    result = CHECK_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_NS_INTERVAL)?TRUE:FALSE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return result;
}

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
BOOL_T NETCFG_OM_ND_SetGlobalNdHoplimit(UI32_T hoplimit)
{
    UI32_T     original_priority;
    int i, total = sizeof(nd_cfg) / sizeof(nd_cfg[0]);
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    for(i=0;i<total;i++)
    {
        nd_cfg[i].ra_hoplimit = hoplimit;
        SET_FLAG(nd_cfg[i].flag, NETCFG_OM_ND_FLAG_ISSET_RA_HOPLIMIT);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_SetNdHoplimit(UI32_T vid_ifIndex,UI32_T hoplimit)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    ndcfg->ra_hoplimit = hoplimit;
    SET_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_RA_HOPLIMIT);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_UnsetGlobalNdHoplimit()
{
    UI32_T     original_priority;
    int i, total = sizeof(nd_cfg) / sizeof(nd_cfg[0]);
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    for(i=0;i<total;i++)
    {
        nd_cfg[i].ra_hoplimit = SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_HOPLIMIT;
        UNSET_FLAG(nd_cfg[i].flag, NETCFG_OM_ND_FLAG_ISSET_RA_HOPLIMIT);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_UnsetNdHoplimit(UI32_T vid_ifIndex)

{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    ndcfg->ra_hoplimit = SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_HOPLIMIT;
    UNSET_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_RA_HOPLIMIT);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_GetGlobalNdHoplimit(UI32_T *hoplimit)
{//hop limit command is global , so we use only 1st to represent global configuration
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= &nd_cfg[0];
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    *hoplimit = ndcfg->ra_hoplimit;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_GetNdHoplimit(UI32_T vid_ifIndex,UI32_T *hoplimit)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    *hoplimit = ndcfg->ra_hoplimit;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_IsSetNdHoplimit(UI32_T vid_ifIndex)
{
    UI32_T     original_priority;
    BOOL_T result;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    result = CHECK_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_RA_HOPLIMIT)?TRUE:FALSE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return result;
}
BOOL_T NETCFG_OM_ND_IsSetGlobalNdHoplimit()
{//hop limit command is global , so we use only 1st to represent global configuration
    UI32_T     original_priority;
    BOOL_T result;
    NETCFG_OM_ND_CONFIG_T* ndcfg= &nd_cfg[0];
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    result = CHECK_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_RA_HOPLIMIT)?TRUE:FALSE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);
    return result;
}

/* FUNCTION NAME : NETCFG_OM_ND_GetNdPrefix
 * PURPOSE:
 *    get  which IPv6 prefixes are included in IPv6 router advertisements
 * INPUT:
 *     L_INET_AddrIp_T: prefix
 *    UI32_T vid_ifIndex ,  validLifetime, preferredLifetime
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
BOOL_T NETCFG_OM_ND_SetNdPrefix(UI32_T vid_ifIndex, L_INET_AddrIp_T *prefix, UI32_T  validLifetime, UI32_T  preferredLifetime,BOOL_T enable_on_link,BOOL_T enable_autoconf)
{
    UI32_T                  original_priority;
    NETCFG_OM_ND_PREFIX_T   ra_prefix;
    NETCFG_OM_ND_CONFIG_T   *ndcfg_p= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    BOOL_T                  result;

    if(ndcfg_p == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    if(prefix == NULL)
    {
        DBGprintf("Null input: prefix");
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    ra_prefix.ifIndex = vid_ifIndex;
    ra_prefix.prefix = *prefix;
    ra_prefix.vlifetime = validLifetime;
    ra_prefix.plifetime = preferredLifetime;
    ra_prefix.flags=0;
    ra_prefix.flags |= ((enable_on_link)?NETCFG_OM_ND_FLAG_ON_LINK:0) ;
    ra_prefix.flags |= ((enable_autoconf)?NETCFG_OM_ND_FLAG_AUTO_ADDRESS:0) ;
    result =L_SORT_LST_Set(&ndcfg_p->ra_prefix_list,&ra_prefix);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return result;
}
BOOL_T NETCFG_OM_ND_GetNdPrefix(UI32_T vid_ifIndex, L_INET_AddrIp_T *prefix, UI32_T *validLifetime, UI32_T *preferredLifetime,BOOL_T*enable_on_link, BOOL_T *enable_autoconf)
{
    NETCFG_OM_ND_CONFIG_T* cfgp = NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    UI32_T     original_priority;
    NETCFG_OM_ND_PREFIX_T ra_prefix;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    BOOL_T result;
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    if(prefix == NULL)
    {
        DBGprintf("Null input: prefix");
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    ra_prefix.ifIndex = vid_ifIndex;
    ra_prefix.prefix = *prefix;
    ra_prefix.vlifetime = 0;
    ra_prefix.plifetime = 0;
    ra_prefix.flags=0;

    result= L_SORT_LST_Get(&cfgp->ra_prefix_list,&ra_prefix);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);
    *validLifetime = ra_prefix.vlifetime;
    *preferredLifetime = ra_prefix.plifetime;
    *enable_on_link =(ra_prefix.flags & NETCFG_OM_ND_FLAG_ON_LINK)?TRUE:FALSE;
    *enable_autoconf =(ra_prefix.flags & NETCFG_OM_ND_FLAG_AUTO_ADDRESS)?TRUE:FALSE;
    return result;
}
BOOL_T NETCFG_OM_ND_GetNextNdPrefix(UI32_T vid_ifIndex, L_INET_AddrIp_T *prefix, UI32_T *validLifetime, UI32_T *preferredLifetime,BOOL_T *enable_on_link, BOOL_T *enable_autoconf)
{
    NETCFG_OM_ND_CONFIG_T   *ndcfg = NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    UI32_T                  original_priority;
    NETCFG_OM_ND_PREFIX_T   ra_prefix;
    L_INET_AddrIp_T         zero_prefix;
    BOOL_T                  result;

    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    if(prefix == NULL)
    {
        DBGprintf("Null input: prefix");
        return FALSE;
    }
    memset(&zero_prefix,0,sizeof(zero_prefix));

    ra_prefix.ifIndex = vid_ifIndex;
    ra_prefix.prefix = *prefix;
    ra_prefix.vlifetime = 0;
    ra_prefix.plifetime = 0;
    ra_prefix.flags=0;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    if(memcmp(&zero_prefix,prefix,sizeof(zero_prefix))==0)
    {
        result = L_SORT_LST_Get_1st(&ndcfg->ra_prefix_list, &ra_prefix);
    }
    else
    {
        result= L_SORT_LST_Get_Next(&ndcfg->ra_prefix_list, &ra_prefix);
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    if (TRUE == result)
    {
        *validLifetime = ra_prefix.vlifetime;
        *preferredLifetime = ra_prefix.plifetime;
        *enable_on_link =(ra_prefix.flags & NETCFG_OM_ND_FLAG_ON_LINK)?TRUE:FALSE;
        *enable_autoconf =(ra_prefix.flags & NETCFG_OM_ND_FLAG_AUTO_ADDRESS)?TRUE:FALSE;
        *prefix = ra_prefix.prefix;
    }
    return result;
}
BOOL_T NETCFG_OM_ND_UnsetNdPrefix(UI32_T vid_ifIndex, L_INET_AddrIp_T *prefix)
{
    NETCFG_OM_ND_CONFIG_T* ndcfg = NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    UI32_T     original_priority;
    NETCFG_OM_ND_PREFIX_T ra_prefix;
    BOOL_T result;
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    if(prefix == NULL)
    {
        DBGprintf("Null input: prefix");
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    ra_prefix.ifIndex = vid_ifIndex;
    ra_prefix.prefix = *prefix;
    ra_prefix.vlifetime = 0;
    ra_prefix.plifetime = 0;
    ra_prefix.flags=0;

    result= L_SORT_LST_Delete(&ndcfg->ra_prefix_list,&ra_prefix);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);
    return result;
}
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
BOOL_T NETCFG_OM_ND_SetNdManagedConfigFlag(UI32_T vid_ifIndex, BOOL_T  enableFlag)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    if(enableFlag)
        SET_FLAG(ndcfg->flag ,NETCFG_OM_ND_FLAG_MANAGED);
    else
        UNSET_FLAG(ndcfg->flag ,NETCFG_OM_ND_FLAG_MANAGED);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_GetNdManagedConfigFlag(UI32_T vid_ifIndex, BOOL_T *enableFlag)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);

    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    *enableFlag= CHECK_FLAG(ndcfg->flag ,NETCFG_OM_ND_FLAG_MANAGED)?TRUE:FALSE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}

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
BOOL_T NETCFG_OM_ND_SetNdOtherConfigFlag(UI32_T vid_ifIndex, BOOL_T  enableFlag)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    if(enableFlag)
        SET_FLAG(ndcfg->flag ,NETCFG_OM_ND_FLAG_OTHER);
    else
        UNSET_FLAG(ndcfg->flag ,NETCFG_OM_ND_FLAG_OTHER);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_GetNdOtherConfigFlag(UI32_T vid_ifIndex, BOOL_T *enableFlag)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    *enableFlag= CHECK_FLAG(ndcfg->flag ,NETCFG_OM_ND_FLAG_OTHER)?TRUE:FALSE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
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
BOOL_T NETCFG_OM_ND_SetNdReachableTime(UI32_T vid_ifIndex,UI32_T  msec)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    ndcfg->reachable_time = msec;
    SET_FLAG(ndcfg->flag,NETCFG_OM_ND_FLAG_ISSET_REACHABLE_TIME);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_UnsetNdReachableTime(UI32_T vid_ifIndex)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    ndcfg->reachable_time = SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_REACHABLE_TIME;
    UNSET_FLAG(ndcfg->flag,NETCFG_OM_ND_FLAG_ISSET_REACHABLE_TIME);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_GetNdReachableTime(UI32_T vid_ifIndex,UI32_T *msec)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    *msec = ndcfg->reachable_time;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_IsSetNdReachableTime(UI32_T vid_ifIndex)
{
    UI32_T     original_priority;
    BOOL_T result;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    result = CHECK_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_REACHABLE_TIME)?TRUE:FALSE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return result;
}
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
BOOL_T NETCFG_OM_ND_SetNdRaSuppress(UI32_T vid_ifIndex, BOOL_T  enableSuppress)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    if(enableSuppress)
        SET_FLAG(ndcfg->flag ,NETCFG_OM_ND_FLAG_SUPPRESS);
    else
        UNSET_FLAG(ndcfg->flag ,NETCFG_OM_ND_FLAG_SUPPRESS);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_GetNdRaSuppress(UI32_T vid_ifIndex, BOOL_T *enableSuppress)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    *enableSuppress= (ndcfg->flag&NETCFG_OM_ND_FLAG_SUPPRESS)?TRUE:FALSE;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
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
BOOL_T NETCFG_OM_ND_SetNdRaLifetime(UI32_T vid_ifIndex, UI32_T  seconds)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    ndcfg->ra_lifetime = seconds;

    if (seconds != SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_ROUTER_LIFETIME)
        SET_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_RA_LIFETIME);
    else
        UNSET_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_RA_LIFETIME);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_UnsetNdRaLifetime(UI32_T vid_ifIndex)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    ndcfg->ra_lifetime = SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_ROUTER_LIFETIME;
    UNSET_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_RA_LIFETIME);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_GetNdRaLifetime(UI32_T vid_ifIndex, UI32_T *seconds)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    *seconds = ndcfg->ra_lifetime;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_IsSetNdRaLifetime(UI32_T vid_ifIndex)
{
    UI32_T     original_priority;
    BOOL_T result;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    result = CHECK_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_RA_LIFETIME)?TRUE:FALSE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return result;
}


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
BOOL_T NETCFG_OM_ND_SetNdRaInterval(UI32_T vid_ifIndex, UI32_T max, UI32_T min)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    ndcfg->max_ra_interval = max;
    ndcfg->min_ra_interval = min;

    if (max != SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_MAX_TRANSMISSIONS_INTERVAL ||
        min != SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_MIN_TRANSMISSIONS_INTERVAL)
        SET_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_RA_INTERVAL);
    else
        UNSET_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_RA_INTERVAL);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_UnsetNdRaInterval(UI32_T vid_ifIndex)
{
    UI32_T     original_priority;

    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    ndcfg->max_ra_interval = SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_MAX_TRANSMISSIONS_INTERVAL;
    ndcfg->min_ra_interval = SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_MIN_TRANSMISSIONS_INTERVAL;
    UNSET_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_RA_INTERVAL);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}

BOOL_T NETCFG_OM_ND_GetNdRaInterval(UI32_T vid_ifIndex, UI32_T *max_p, UI32_T *min_p)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    *max_p = ndcfg->max_ra_interval;
    *min_p = ndcfg->min_ra_interval;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_IsSetNdRaInterval(UI32_T vid_ifIndex)
{
    UI32_T     original_priority;
    BOOL_T result;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    result = CHECK_FLAG(ndcfg->flag, NETCFG_OM_ND_FLAG_ISSET_RA_INTERVAL)?TRUE:FALSE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return result;
}

/* FUNCTION NAME : NETCFG_OM_ND_GetNdRouterPreference
 * PURPOSE:
 *    get the the Preference flag in IPv6 router advertisements
 * INPUT:
 *    vid_ifIndex
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
BOOL_T NETCFG_OM_ND_SetNdRouterPreference(UI32_T vid_ifIndex, UI32_T   prefer)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    ndcfg->preference = prefer;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}
BOOL_T NETCFG_OM_ND_GetNdRouterPreference(UI32_T vid_ifIndex, UI32_T  *prefer)
{
    UI32_T     original_priority;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);

    *prefer=ndcfg->preference ;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}

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

BOOL_T NETCFG_OM_ND_InitNdConfig(UI32_T vid_ifIndex)
{
    NETCFG_OM_ND_CONFIG_T* cfgp = NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(cfgp == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifIndex);
        return FALSE;
    }
    cfgp->ra_hoplimit = SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_HOPLIMIT;
    cfgp->dad_attempts = SYS_DFLT_ND_DUPLICATE_ADDRESS_DETECTION_ATTEMPTS;
    cfgp->ns_interval = SYS_DFLT_ND_NEIGHBOR_SOLICITATION_RETRANSMISSIONS_INTERVAL;
    cfgp->reachable_time= SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_REACHABLE_TIME;
    cfgp->ra_lifetime= SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_ROUTER_LIFETIME;/* rfc4681:AdvDefaultLifetime*/
    cfgp->max_ra_interval= SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_MAX_TRANSMISSIONS_INTERVAL;
    cfgp->min_ra_interval= SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_MIN_TRANSMISSIONS_INTERVAL;
    cfgp->preference= SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_DEFAULT_ROUTER_PREFERENCE;
    cfgp->flag =0;
    if(L_SORT_LST_Create(&cfgp->ra_prefix_list, /*prefix number ??*/16, sizeof(NETCFG_OM_ND_PREFIX_T), NETCFG_OM_ND_RAPrefixCompare)==FALSE)
    {
        /* DEBUG */
        DBGprintf("fail to call L_SORT_LST_Create(,%d,)",(int)sizeof(NETCFG_OM_ND_PREFIX_T));
        SYSFUN_LogDebugMsg("NETCFG_OM_ND_Init : Can't create prefix List.\n");
    }
    //make flags
    cfgp->flag =0;
    if(SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_MANAGED_ADDRESS)
    {
        SET_FLAG(cfgp->flag ,NETCFG_OM_ND_FLAG_MANAGED);
    }
    if(SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_OTHER_CONFIG)
    {
        SET_FLAG(cfgp->flag ,NETCFG_OM_ND_FLAG_OTHER);
    }
    if(SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_SUPPRESS)
    {
        SET_FLAG(cfgp->flag ,NETCFG_OM_ND_FLAG_SUPPRESS);
    }

    return TRUE;

}

BOOL_T NETCFG_OM_ND_IsConfigFlagSet(UI32_T vid_ifindex, UI32_T flag, BOOL_T *is_set)
{
    UI32_T original_priority;
    BOOL_T result;
    NETCFG_OM_ND_CONFIG_T* ndcfg= NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifindex);
    if(ndcfg == NULL)
    {
        DBGprintf("Fail to call NETCFG_OM_ND_GetNdconfigByIfindex(%ld)", (long)vid_ifindex);
        return FALSE;
    }
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
    *is_set = CHECK_FLAG(ndcfg->flag, flag)?TRUE:FALSE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

    return TRUE;
}

#endif/* #if (SYS_CPNT_IPV6 == TRUE)*/

void NETCFG_OM_ND_BackdoorDumpDB(UI32_T vid_ifIndex)
{
    NETCFG_OM_ND_CONFIG_T* cfgp;
    NETCFG_OM_ND_PREFIX_T ra_prefix;
    char strbuff[L_INET_MAX_IP6ADDR_STR_LEN+1];
    NETCFG_TYPE_StaticIpNetToMediaEntry_T    v4entry;
    NETCFG_TYPE_StaticIpv6NetToMediaEntry_T v6entry;
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T entry;

    printf("\r\ndump database of vlan %d\r\n", vid_ifIndex);
    printf("ipv4_arp_timeout=%ld\r\n", (long)ipv4_arp_timeout);

    cfgp = NETCFG_OM_ND_GetNdconfigByIfindex(vid_ifIndex);
    if(NULL != cfgp )
    {
        printf("ns_interval=%ld\r\n", (long)cfgp->ns_interval);
        printf("dad_attempts=%ld\r\n", (long)cfgp->dad_attempts);
        printf("valid_lifetime=%ld\r\n", (long)cfgp->valid_lifetime);
        printf("reachable_time=%ld\r\n", (long)cfgp->reachable_time);
        printf("ra_lifetime=%ld\r\n", (long)cfgp->ra_lifetime);
        printf("max_ra_interval=%ld\r\n", (long)cfgp->max_ra_interval);
        printf("min_ra_interval=%ld\r\n", (long)cfgp->min_ra_interval);
        printf("ra_hoplimit=%d\r\n",cfgp->ra_hoplimit);
        printf("flag=%x\r\n",cfgp->flag);
        printf("preference=%d\r\n",cfgp->preference);

        if(L_SORT_LST_Get_1st(&cfgp->ra_prefix_list, &ra_prefix)){
            printf("Dump prefix %ld:\r\n", (long)cfgp->ra_prefix_list.nbr_of_element);
            do{
                printf("prefix.ra_prefix.ifIndex=%ld\r\n", (long)ra_prefix.ifIndex);
                printf("prefix.ra_prefix.flags=%x\r\n", ra_prefix.flags);
                printf("prefix.ra_prefix.vlifetime=%ld\r\n", (long)ra_prefix.vlifetime);
                printf("prefix.ra_prefix.plifetime=%ld\r\n", (long)ra_prefix.plifetime);
                if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&ra_prefix.prefix,
                                                                   strbuff,
                                                                   sizeof(strbuff)))
                    sprintf(strbuff,"UNKNOWNADDR");
                printf("prefix.prefix = %s\r\n",strbuff );


            }while (L_SORT_LST_Get_Next(&cfgp->ra_prefix_list, &ra_prefix));
        }
    }

    memset(&entry,0,sizeof(entry));
    while (NETCFG_OM_ND_GetNextStaticEntry(L_INET_ADDR_TYPE_IPV4, &entry)){
        if (entry.ip_net_to_physical_entry.ip_net_to_physical_if_index!=vid_ifIndex)
            continue;
        printf("static nd.status=%d\r\n", entry.status);
        printf("static nd.ifindex=%ld\r\n", (long)entry.ip_net_to_physical_entry.ip_net_to_physical_if_index);
        printf("static nd.net=%d.%d.%d.%d\r\n", L_INET_EXPAND_IPV4(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr));
        printf("static nd.phy=%2x-%2x-%2x-%2x-%2x-%2x\r\n", L_INET_EXPAND_MAC(entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string));
        printf("static nd.media=%d\r\n", entry.ip_net_to_physical_entry.ip_net_to_physical_type);
    }

    /*if(L_SORT_LST_Get_1st(&neighbors[NETCFG_OM_NDV4], &v4entry)){
        printf("Dump IPV4 static neighbor %ld:\r\n", neighbors[NETCFG_OM_NDV4].nbr_of_element);
        do{
            if (v4entry.ip_net_to_media_entry.ip_net_to_media_if_index!=vid_ifIndex)
                continue;
            printf("static nd.status=%d\r\n", v4entry.status);
            printf("static nd.ifindex=%ld\r\n", v4entry.ip_net_to_media_entry.ip_net_to_media_if_index);
            printf("static nd.net=%d.%d.%d.%d\r\n", L_INET_EXPAND_IPV4(v4entry.ip_net_to_media_entry.ip_net_to_media_net_address));
            printf("static nd.phy=%2x-%2x-%2x-%2x-%2x-%2x\r\n", L_INET_EXPAND_MAC(v4entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string));
            printf("static nd.media=%d\r\n", v4entry.ip_net_to_media_entry.ip_net_to_media_type);

        }while (L_SORT_LST_Get_Next(&neighbors[NETCFG_OM_NDV4], &v4entry));
    }*/
    memset(&entry,0,sizeof(entry));
    while(NETCFG_OM_ND_GetNextStaticEntry(L_INET_ADDR_TYPE_IPV6, &entry)){
            if (entry.ip_net_to_physical_entry.ip_net_to_physical_if_index!=vid_ifIndex)
                continue;
            printf("static nd.status=%d, type=%d\r\n", entry.status,entry.ip_net_to_physical_entry.ip_net_to_physical_type);
            printf("static nd.ifindex=%ld\r\n", (long)entry.ip_net_to_physical_entry.ip_net_to_physical_if_index);

            if(! L_INET_Ntop(L_INET_AF_INET6, entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr, strbuff, sizeof(strbuff)))
                 sprintf(strbuff,"UNKNOWNADDR");
            printf("static nd.net = %s\r\n",strbuff );
            printf("static nd.phy=%2x-%2x-%2x-%2x-%2x-%2x\r\n", L_INET_EXPAND_MAC(entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string));
    }
    /*
     if(L_SORT_LST_Get_1st(&neighbors[NETCFG_OM_NDV6], &v6entry)){
        printf("Dump IPV6 static neighbor %ld:\r\n", neighbors[NETCFG_OM_NDV6].nbr_of_element);
        do{
            if (v6entry.ip_net_to_media_entry.ip_net_to_media_if_index!=vid_ifIndex)
                continue;
            printf("static nd.status=%d, type=%d\r\n", v6entry.status,v6entry.ip_net_to_media_entry.ip_net_to_media_type);
            printf("static nd.ifindex=%ld\r\n", v6entry.ip_net_to_media_entry.ip_net_to_media_if_index);
            strlen= sizeof(strbuff);
            if(! L_INET_AddrToStr6(v6entry.ip_net_to_media_entry.ip_net_to_media_net_address,&strlen, strbuff))
                 sprintf(strbuff,"UNKNOWNADDR");
            printf("static nd.net = %s\r\n",strbuff );
            printf("static nd.phy=%2x-%2x-%2x-%2x-%2x-%2x\r\n", L_INET_EXPAND_MAC(v6entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string));
            printf("static nd.media=%d\r\n", v6entry.ip_net_to_media_entry.ip_net_to_media_type);

        }while (L_SORT_LST_Get_Next(&neighbors[NETCFG_OM_NDV6], &v6entry));
    }
    */

}
void NETCFG_OM_ND_BackdoorSetDebugFlag(UI32_T flag)
{
    DEBUG_FLAG = flag;
}

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)

#define NETCFG_OM_ND_LOCK()     original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(neighbor_lcb.sem_id);
#define NETCFG_OM_ND_UNLOCK()   SYSFUN_OM_LEAVE_CRITICAL_SECTION(neighbor_lcb.sem_id, original_priority);

typedef struct NETCFG_OM_ND_RaGuardCfgEntry_S
{
    UI32_T                              enabled_port_num;
    UI8_T                               enabled_lports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];  /* enabled */
                                        /* statis only count the sw relay packets */
    NETCFG_OM_ND_RaGuardPortStatisEntry_T   statis[SYS_ADPT_TOTAL_NBR_OF_LPORT];
} NETCFG_OM_ND_RaGuardCfgEntry_T;

static void NETCFG_OM_ND_RAGUARD_LocalIncStatistics(
    UI32_T  lport,
    UI32_T  pkt_type,
    UI32_T  recv_cnt,
    UI32_T  drop_cnt);
static BOOL_T NETCFG_OM_ND_RAGUARD_LocalIsEnabled(
    UI32_T  lport);
static void NETCFG_OM_ND_RAGUARD_LocalSetPortStatus(
    UI32_T  lport,
    BOOL_T  is_enable);
static UI32_T                           original_priority;
static NETCFG_OM_ND_RaGuardCfgEntry_T   raguard_cfg;

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
void NETCFG_OM_ND_RAGUARD_ClearOm(void)
{
    memset(&raguard_cfg, 0, sizeof(raguard_cfg));
}

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
BOOL_T NETCFG_OM_ND_RAGUARD_IsAnyPortEnabled(void)
{
    BOOL_T  ret = FALSE;

    NETCFG_OM_ND_LOCK();

    if (raguard_cfg.enabled_port_num >0)
    {
        ret = TRUE;
    }

    NETCFG_OM_ND_UNLOCK();

    return ret;
}

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
    UI32_T  pkt_type)
{
    BOOL_T  ret = FALSE;

    if ((1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        NETCFG_OM_ND_LOCK();
        ret = NETCFG_OM_ND_RAGUARD_LocalIsEnabled(lport);

        if (pkt_type < NETCFG_TYPE_RG_PKT_MAX)
        {
            UI32_T  drop_cnt = (TRUE == ret) ? 1 : 0;

            NETCFG_OM_ND_RAGUARD_LocalIncStatistics(
                lport, pkt_type, 1, drop_cnt);
        }
        NETCFG_OM_ND_UNLOCK();
    }

    return ret;
}

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
    BOOL_T  is_enable)
{
    BOOL_T  ret = FALSE;

    if ((1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        NETCFG_OM_ND_LOCK();
        if (is_enable != NETCFG_OM_ND_RAGUARD_LocalIsEnabled(lport))
        {
            NETCFG_OM_ND_RAGUARD_LocalSetPortStatus(lport, is_enable);
        }
        NETCFG_OM_ND_UNLOCK();
        ret = TRUE;
    }

    return ret;
}

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
    BOOL_T  *is_enable_p)
{
    BOOL_T  ret = FALSE;

    if (  (NULL != is_enable_p)
        &&(1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
       )
    {
        NETCFG_OM_ND_LOCK();
        *is_enable_p = NETCFG_OM_ND_RAGUARD_LocalIsEnabled(lport);
        NETCFG_OM_ND_UNLOCK();
        ret = TRUE;
    }

    return ret;
}

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
    UI32_T  dst_lport)
{
    BOOL_T  ret = FALSE, is_src_enabled, is_dst_enabled;

    if (  (1 <= src_lport) && (src_lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
        &&(1 <= dst_lport) && (dst_lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
       )
    {
        NETCFG_OM_ND_LOCK();

        is_src_enabled = NETCFG_OM_ND_RAGUARD_LocalIsEnabled(src_lport);
        is_dst_enabled = NETCFG_OM_ND_RAGUARD_LocalIsEnabled(dst_lport);

        if (is_src_enabled != is_dst_enabled)
        {
            NETCFG_OM_ND_RAGUARD_LocalSetPortStatus(dst_lport, is_src_enabled);
        }

        NETCFG_OM_ND_UNLOCK();
        ret = TRUE;
    }

    return ret;
}

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
    NETCFG_OM_ND_RaGuardPortStatisEntry_T   *pstat_p)
{
    BOOL_T  ret = FALSE;

    if (  (NULL != pstat_p)
        &&(1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
       )
    {
        NETCFG_OM_ND_LOCK();
        memcpy(pstat_p, &raguard_cfg.statis[lport-1], sizeof(*pstat_p));
        NETCFG_OM_ND_UNLOCK();
        ret = TRUE;
    }

    return ret;
}

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
    UI32_T  drop_cnt)
{
    if (  (pkt_type < NETCFG_TYPE_RG_PKT_MAX)
        &&(1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
       )
    {
        NETCFG_OM_ND_LOCK();
        NETCFG_OM_ND_RAGUARD_LocalIncStatistics(
            lport, pkt_type, recv_cnt, drop_cnt);
        NETCFG_OM_ND_UNLOCK();
    }
}

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
void NETCFG_OM_ND_RAGUARD_BackdoorDumpDB(void)
{
    UI32_T  idx;

    BACKDOOR_MGR_Printf("RA Guard Config\r\n");
    BACKDOOR_MGR_Printf(" Enabled Port Num: %ld\r\n", (long)raguard_cfg.enabled_port_num);
    BACKDOOR_MGR_Printf(" %5s %7s %10s %10s %10s %10s\r\n",
        "LPORT", "ENABLED", "RA RECV", "RA DROP", "RR RECV", "RR DROP");

    for (idx =0; idx<SYS_ADPT_TOTAL_NBR_OF_LPORT; idx++)
    {
        BACKDOOR_MGR_Printf(" %5d %7c %10ld %10ld %10ld %10ld\r\n",
            idx+1,
            (TRUE == NETCFG_OM_ND_RAGUARD_IsEnabled(
                        idx+1, NETCFG_TYPE_RG_PKT_MAX)) ? 'Y' : 'N',
            (long)raguard_cfg.statis[idx].recv_cnt[0],
            (long)raguard_cfg.statis[idx].drop_cnt[0],
            (long)raguard_cfg.statis[idx].recv_cnt[1],
            (long)raguard_cfg.statis[idx].drop_cnt[1]);
    }
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_RAGUARD_LocalIncStatistics
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
static void NETCFG_OM_ND_RAGUARD_LocalIncStatistics(
    UI32_T  lport,
    UI32_T  pkt_type,
    UI32_T  recv_cnt,
    UI32_T  drop_cnt)
{
    raguard_cfg.statis[lport-1].recv_cnt[pkt_type] += recv_cnt;
    raguard_cfg.statis[lport-1].drop_cnt[pkt_type] += drop_cnt;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_RAGUARD_LocalIsEnabled
 * ------------------------------------------------------------------------
 * PURPOSE: To check if RA Guard is enabled for specifed lport.
 * INPUT  : lport - which lport to check (1-based)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
static BOOL_T NETCFG_OM_ND_RAGUARD_LocalIsEnabled(
    UI32_T  lport)
{
    UI32_T  lport_byte;
    UI8_T   lport_byte_mask;
    BOOL_T  ret = FALSE;

    lport_byte_mask = (1 << (7 - ((lport - 1) & 7)));
    lport_byte      = (lport - 1) >> 3;

    if (0 != (raguard_cfg.enabled_lports[lport_byte] & lport_byte_mask))
    {
        ret = TRUE;
    }

    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ND_RAGUARD_LocalSetPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To set RA Guard port status for specifed lport.
 * INPUT  : lport     - which lport to set (1-based)
 *          is_enable - TRUE to enable
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
static void NETCFG_OM_ND_RAGUARD_LocalSetPortStatus(
    UI32_T  lport,
    BOOL_T  is_enable)
{
    UI32_T  lport_byte;
    UI8_T   lport_byte_mask;

    lport_byte_mask = (1 << (7 - ((lport - 1) & 7)));
    lport_byte      = (lport - 1) >> 3;

    if (TRUE == is_enable)
    {
        raguard_cfg.enabled_lports[lport_byte] |= lport_byte_mask;
        raguard_cfg.enabled_port_num++;
    }
    else
    {
        raguard_cfg.enabled_lports[lport_byte] &= ~lport_byte_mask;

        if (raguard_cfg.enabled_port_num > 0)
            raguard_cfg.enabled_port_num--;
    }
}


#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

