/*-----------------------------------------------------------------------------
 * FILE NAME: ARP_MGR.C
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
#include "sys_type.h"
#include "sys_bld.h"
#include "netcfg_type.h"
#include "sysfun.h"
#include "sys_module.h"
#include "sys_dflt.h"
#include "l_mm.h"
#include "string.h"
#include "stdlib.h"
#include "l_stdlib.h"
#include "netcfg_netdevice.h"
#if (SYS_CPNT_AMTRL3 == TRUE)
#include "amtrl3_type.h"
#include "amtrl3_pom.h"
#include "amtrl3_pmgr.h"
#endif
#include "leaf_4001.h"
#include "vlan_lib.h"
#include "netcfg_netdevice.h"
#if (SYS_CPNT_NSM == TRUE)
#include "nsm_pmgr.h"
#endif
#include "nd_mgr.h"
#include "netcfg_type.h"
#include "ipal_types.h"
#include "ipal_icmp.h"
#include "ipal_if.h"
#include "ipal_neigh.h"
#include "netcfg_om_nd.h"
#include "netcfg_om_ip.h"
#include "vlan_pom.h"
#include "l_hisam.h"

#define SN_LAN                          1

#if (SYS_CPNT_IPAL == TRUE)
static  IPAL_NeighborEntry_T                    *_IPAL_NeighborEntry_T; /* for ND_MGR_SIZEOF use */
#define ND_MGR_HOST_ROUTE_CACHE_NBR             512
#define ND_MGR_HOST_ROUTE_CACHE_INDEX_NBR       ((ND_MGR_HOST_ROUTE_CACHE_NBR * 2 / ND_MGR_HOST_ROUTE_CACHE_HISAM_N2) + 1)
#define ND_MGR_HOST_ROUTE_CACHE_HISAM_N1        4
#define ND_MGR_HOST_ROUTE_CACHE_HISAM_N2        (ND_MGR_HOST_ROUTE_CACHE_NBR / 32 + 8)
#define ND_MGR_HOST_ROUTE_CACHE_HASH_DEPTH      4
#define ND_MGR_HOST_ROUTE_CACHE_HASH_NBR        (ND_MGR_HOST_ROUTE_CACHE_NBR / 4 + 16)
#define ND_MGR_HOST_ROUTE_CACHE_REFRESH_TIME    1000 /* 10 seconds in ticks */
#define ND_MGR_OFFSET(offset, type, field)      { type v; offset=(UI8_T *)&v.field - (UI8_T *)&v; }
#define ND_MGR_SIZEOF(type, field)              sizeof (_##type->field)
#define ND_MGR_SIZEOF_HOST_ROUTE_IFINDEX        ND_MGR_SIZEOF(IPAL_NeighborEntry_T, ifindex)
#define ND_MGR_SIZEOF_HOST_ROUTE_IPADDR         ND_MGR_SIZEOF(IPAL_NeighborEntry_T, ip_addr)
#define ND_MGR_HOST_ROUTE_CACHE_KEY_LEN         (ND_MGR_SIZEOF_HOST_ROUTE_IFINDEX + ND_MGR_SIZEOF_HOST_ROUTE_IPADDR)
#define ND_MGR_HOST_ROUTE_CACHE_KIDX            0
#endif

 /* debug function*/
#define DEBUG_FLAG_BIT_DEBUG 0x01
#define DEBUG_FLAG_BIT_INFO  0x02
#define DEBUG_FLAG_BIT_NOTE  0x04
/***************************************************************/
static UI32_T DEBUG_FLAG =0; /*DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO|DEBUG_FLAG_BIT_NOTE;*/
/***************************************************************/
#define DBGprintf(format,args...) ((DEBUG_FLAG_BIT_DEBUG & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define INFOprintf(format,args...) ((DEBUG_FLAG_BIT_INFO & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define NOTEprintf(format,args...) ((DEBUG_FLAG_BIT_NOTE & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)

#define FORMAT_MAC %x-%x-%x-%x-%x-%x
#define FORMAT_IPV4 %d.%d.%d.%d
#define FORMAT_IPV6 %X:%X:%X:%X:%X:%X:%X:%X

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

#if (SYS_CPNT_IPAL == TRUE)
L_HISAM_Desc_T          nd_mgr_host_route_cache_hisam_desc = {0};
UI32_T                  nd_mgr_host_route_cache_last_upate = 0;
IPAL_NeighborEntry_T    nd_mgr_host_route_cache_ipal_neighbor[ND_MGR_HOST_ROUTE_CACHE_NBR];
NETCFG_TYPE_IpNetToPhysicalEntry_T nd_mgr_host_route_cache_last_key;
#endif

void ND_MGR_BackdoorSetDebugFlag(UI32_T flag)
{
    DEBUG_FLAG = flag;
}

/* FUNCTION NAME : ND_MGR_InitiateProcessResources
 * PURPOSE: Initialize process resources for ARP_MGR
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
void ND_MGR_InitiateProcessResources(void)
{
#if (SYS_CPNT_IPV6 == TRUE)
    if(IPAL_RESULT_OK != IPAL_ICMP_SetIpv6NeighDefaultRetransTime(SYS_DFLT_ND_NEIGHBOR_SOLICITATION_RETRANSMISSIONS_INTERVAL))
    {
        DBGprintf("init fail for IPAL_ICMP_SetIpv6NeighDefaultRetransTime()");
    }
    if(IPAL_RESULT_OK != IPAL_ICMP_SetIpv6NeighDefaultReachableTime(SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_REACHABLE_TIME))
    {
        DBGprintf("init fail for IPAL_ICMP_SetIpv6NeighDefaultReachableTime()");
    }
    if(IPAL_RESULT_OK!= IPAL_IF_SetDefaultHopLimit(SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_HOPLIMIT))
    {
        DBGprintf("init fail for IPAL_IF_SetDefaultHopLimit()");
    }
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

#if (SYS_CPNT_IPAL == TRUE)
{
    L_HISAM_KeyDef_T key_def_table[1];

    key_def_table[ND_MGR_HOST_ROUTE_CACHE_KIDX].field_number = 2;
    ND_MGR_OFFSET(key_def_table[ND_MGR_HOST_ROUTE_CACHE_KIDX].offset[0],IPAL_NeighborEntry_T, ifindex);
    ND_MGR_OFFSET(key_def_table[ND_MGR_HOST_ROUTE_CACHE_KIDX].offset[1],IPAL_NeighborEntry_T, ip_addr);
    key_def_table[ND_MGR_HOST_ROUTE_CACHE_KIDX].length[0] = ND_MGR_SIZEOF_HOST_ROUTE_IFINDEX;
    key_def_table[ND_MGR_HOST_ROUTE_CACHE_KIDX].length[1] = ND_MGR_SIZEOF_HOST_ROUTE_IPADDR;

    nd_mgr_host_route_cache_hisam_desc.total_record_nbr = ND_MGR_HOST_ROUTE_CACHE_NBR;
    nd_mgr_host_route_cache_hisam_desc.total_index_nbr  = ND_MGR_HOST_ROUTE_CACHE_INDEX_NBR;
    nd_mgr_host_route_cache_hisam_desc.total_hash_nbr   = ND_MGR_HOST_ROUTE_CACHE_HASH_NBR;
    nd_mgr_host_route_cache_hisam_desc.record_length    = sizeof(IPAL_NeighborEntry_T);
    nd_mgr_host_route_cache_hisam_desc.hash_depth       = ND_MGR_HOST_ROUTE_CACHE_HASH_DEPTH;
    nd_mgr_host_route_cache_hisam_desc.N1               = ND_MGR_HOST_ROUTE_CACHE_HISAM_N1;
    nd_mgr_host_route_cache_hisam_desc.N2               = ND_MGR_HOST_ROUTE_CACHE_HISAM_N2;

    if (FALSE == L_HISAM_Create(&nd_mgr_host_route_cache_hisam_desc, 1, key_def_table))
    {
        DBGprintf("L_HISAM_Create failed.\n");
    }
}
#endif

    return;
}

/* FUNCTION NAME : ND_MGR_GetIpNetToPhysicalEntry
 * PURPOSE:
 *      Get an arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T     ND_MGR_GetIpNetToPhysicalEntry(NETCFG_TYPE_IpNetToPhysicalEntry_T *entry)
{
#if (SYS_CPNT_AMTRL3 == TRUE)
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    AMTRL3_TYPE_ipNetToPhysicalEntry_T amtrl3_entry;
    UI8_T addrlen,al3_actionflag;

    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    if(entry->ip_net_to_physical_net_address.type== L_INET_ADDR_TYPE_IPV4 ||
        entry->ip_net_to_physical_net_address.type== L_INET_ADDR_TYPE_IPV4Z)
    {
        addrlen = SYS_ADPT_IPV4_ADDR_LEN;
        al3_actionflag =AMTRL3_TYPE_FLAGS_IPV4;
    }
    else if (entry->ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV6 ||
                entry->ip_net_to_physical_net_address.type == L_INET_ADDR_TYPE_IPV6Z)
    {
        addrlen = SYS_ADPT_IPV6_ADDR_LEN;
        al3_actionflag =AMTRL3_TYPE_FLAGS_IPV6;
    }
    else
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    memset(&amtrl3_entry, 0, sizeof(amtrl3_entry));
    amtrl3_entry.ip_net_to_physical_if_index = entry->ip_net_to_physical_if_index;
    amtrl3_entry.ip_net_to_physical_net_address = entry->ip_net_to_physical_net_address;
    if(AMTRL3_POM_GetIpNetToPhysicalEntry(al3_actionflag,fib_id,&amtrl3_entry)!= TRUE)
        return NETCFG_TYPE_CAN_NOT_GET;


    entry->ip_net_to_physical_if_index = amtrl3_entry.ip_net_to_physical_if_index;
    memcpy(&entry->ip_net_to_physical_net_address, amtrl3_entry.ip_net_to_physical_net_address.addr , addrlen);
    entry->ip_net_to_physical_type = (int)(amtrl3_entry.ip_net_to_physical_type);
    entry->ip_net_to_physical_phys_address.phy_address_type = (UI32_T)(amtrl3_entry.ip_net_to_physical_phys_address.phy_address_type);
    entry->ip_net_to_physical_phys_address.phy_address_len = (UI32_T)(amtrl3_entry.ip_net_to_physical_phys_address.phy_address_len);
    memcpy(entry->ip_net_to_physical_phys_address.phy_address_cctet_string, amtrl3_entry.ip_net_to_physical_phys_address.phy_address_octet_string,entry->ip_net_to_physical_phys_address.phy_address_len);
    return NETCFG_TYPE_OK;

#else /* #if (SYS_CPNT_AMTRL3 == TRUE) */

#if (SYS_CPNT_IPAL == TRUE)
    IPAL_NeighborEntry_T ipal_neighbor;

    if(IPAL_RESULT_OK ==  IPAL_NEIGH_GetNeighbor(entry->ip_net_to_physical_if_index,
                                                 &entry->ip_net_to_physical_net_address,
                                                 &ipal_neighbor))
    {
        entry->ip_net_to_physical_last_update = ipal_neighbor.last_update;
        entry->ip_net_to_physical_phys_address.phy_address_type = SN_LAN;
        entry->ip_net_to_physical_phys_address.phy_address_len = ipal_neighbor.phy_address_len;
        memcpy(entry->ip_net_to_physical_phys_address.phy_address_cctet_string, ipal_neighbor.phy_address, ipal_neighbor.phy_address_len);

        /*TODO: For Linux kernel, how to identify local ARP */
        if (ipal_neighbor.state & IPAL_NEIGH_NUD_PERMANENT)
            entry->ip_net_to_physical_type = VAL_ipNetToMediaType_static;
        else
            entry->ip_net_to_physical_type = VAL_ipNetToMediaType_dynamic;

        if (ipal_neighbor.state & IPAL_NEIGH_NUD_REACHABLE)
            entry->ip_net_to_physical_state = NETCFG_TYPE_ND_NEIGHBOR_STATE_REACHABLE;
        else if (ipal_neighbor.state & IPAL_NEIGH_NUD_STALE)
            entry->ip_net_to_physical_state = NETCFG_TYPE_ND_NEIGHBOR_STATE_STALE;
        else if (ipal_neighbor.state & IPAL_NEIGH_NUD_DELAY)
            entry->ip_net_to_physical_state = NETCFG_TYPE_ND_NEIGHBOR_STATE_DELAY;
        else if (ipal_neighbor.state & IPAL_NEIGH_NUD_PROBE)
            entry->ip_net_to_physical_state = NETCFG_TYPE_ND_NEIGHBOR_STATE_PROBE;
        else
            entry->ip_net_to_physical_state = NETCFG_TYPE_ND_NEIGHBOR_STATE_UNKNOWN;

        return NETCFG_TYPE_OK;
    }

    return NETCFG_TYPE_CAN_NOT_GET;

#else /* #if (SYS_CPNT_IPAL == TRUE) */

    return NETCFG_TYPE_FAIL;

#endif /* #if (SYS_CPNT_IPAL == TRUE) */

#endif /* #if (SYS_CPNT_AMTRL3 == TRUE) */
}

/* FUNCTION NAME : ND_MGR_AddStaticIpNetToPhysicalEntry
 * PURPOSE:
 *      create ipv6 neighbor entry
 *
 * INPUT:
 *      UI32_T vid_ifIndex: vlan ifindex
 *      L_INET_AddrIp_T *ip_addr: ipv6 address
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T ND_MGR_AddStaticIpNetToPhysicalEntry(UI32_T vid_ifIndex,
                                            L_INET_AddrIp_T*  ip_addr,
                                            UI32_T phy_addr_len,
                                            UI8_T *phy_addr)
{
#if (SYS_CPNT_AMTRL3 == TRUE)
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    AMTRL3_TYPE_InetHostRouteEntry_T amtrl3_entry;
    UI32_T type = VAL_ipNetToPhysicalExtType_static;
    UI32_T al3_action_flag;

    if(phy_addr == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    //end paremeter check

    if(L_INET_ADDR_TYPE_IPV4 == ip_addr->type ||
       L_INET_ADDR_TYPE_IPV4Z== ip_addr->type)
    {
        al3_action_flag = AMTRL3_TYPE_FLAGS_IPV4;
    }
    else if (L_INET_ADDR_TYPE_IPV6 == ip_addr->type ||
             L_INET_ADDR_TYPE_IPV6Z== ip_addr->type)
    {
        al3_action_flag = AMTRL3_TYPE_FLAGS_IPV6;
    }
    else
    {
        DBGprintf("unknown address type");
        return NETCFG_TYPE_INVALID_ARG;
    }

    memset(&amtrl3_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    amtrl3_entry.dst_vid_ifindex = vid_ifIndex;
    amtrl3_entry.dst_inet_addr = *ip_addr;
    memcpy(amtrl3_entry.dst_mac, phy_addr, phy_addr_len);

    if (AMTRL3_PMGR_SetHostRoute(al3_action_flag, fib_id, &amtrl3_entry, type) != TRUE)
    {
        DBGprintf("Fail to set to AL3:mac= %x-%x-%x-%x-%x-%x",L_INET_EXPAND_MAC(phy_addr));
        return NETCFG_TYPE_CAN_NOT_ADD;
    }

#endif /*SYS_CPNT_AMTRL3*/

#if (SYS_CPNT_IPAL == TRUE)
{
    UI32_T ret;
    ret = IPAL_NEIGH_AddNeighbor(vid_ifIndex, ip_addr, phy_addr_len, phy_addr, IPAL_NEIGHBOR_TYPE_STATIC, TRUE);
    if (ret != IPAL_RESULT_OK && ret != IPAL_RESULT_ENTRY_EXIST)
    {
        DBGprintf("Fail to set to TCP/IP:mac= %x-%x-%x-%x-%x-%x",L_INET_EXPAND_MAC(phy_addr));
        return NETCFG_TYPE_CAN_NOT_ADD;
    }
}
#endif /*SYS_CPNT_IPAL*/

/* if both AMTRL3 and IPAL are not enabled, return fail
 */
#if ((SYS_CPNT_AMTRL3 == TRUE) || (SYS_CPNT_IPAL == TRUE))
    return NETCFG_TYPE_OK;
#else
    return NETCFG_TYPE_FAIL;
#endif
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_DeleteStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete a static ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_DeleteStaticIpNetToMediaEntry(UI32_T vid_ifIndex, UI32_T ip_addr)
{
#if (SYS_CPNT_AMTRL3 == TRUE)
    UI32_T      fib_id = SYS_ADPT_DEFAULT_FIB;
    AMTRL3_TYPE_InetHostRouteEntry_T amtrl3_entry;

    memset(&amtrl3_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));

    amtrl3_entry.dst_vid_ifindex = vid_ifIndex;
    amtrl3_entry.dst_inet_addr.type = VAL_InetAddressType_ipv4;
    memcpy(amtrl3_entry.dst_inet_addr.addr, &ip_addr, sizeof(UI8_T)*4);

    if (AMTRL3_PMGR_DeleteHostRoute(AMTRL3_TYPE_FLAGS_IPV4, fib_id, &amtrl3_entry) != TRUE)
        return NETCFG_TYPE_CAN_NOT_DELETE;
    else
        return NETCFG_TYPE_OK;
#else
#if 0
    if(IPAL_PMGR_DeleteStaticIpNetToMediaEntry() != TRUE)
        return NETCFG_TYPE_CAN_NOT_DELETE;
    else
        return NETCFG_TYPE_OK;
#else
        printf("%s,%d\n",__FUNCTION__,__LINE__);
        return NETCFG_TYPE_OK;
#endif
#endif
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetIpNetToMediaEntryTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE : Set ARP timeout.
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
UI32_T ND_MGR_SetIpNetToMediaEntryTimeout(UI32_T age_time, UI32_T pre_timeout)
{
#if (SYS_CPNT_AMTRL3 == TRUE)
    UI32_T  fib_id = SYS_ADPT_DEFAULT_FIB;

    if (AMTRL3_PMGR_SetIpNetToPhysicalEntryTimeout(AMTRL3_TYPE_FLAGS_IPV4, fib_id, age_time, 0) != TRUE)
        return NETCFG_TYPE_CAN_NOT_SET;

#else
#if (SYS_CPNT_IPAL == TRUE)
    if(IPAL_RESULT_OK != IPAL_NEIGH_SetNeighborAgingTime(age_time))
    {
        return NETCFG_TYPE_CAN_NOT_SET;
    }
#endif/*SYS_CPNT_IPAL*/
#endif
    return NETCFG_TYPE_OK;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_DeleteAllDynamicIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic ARP entries.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_DeleteAllDynamicIpNetToMediaEntry(void)
{
#if (SYS_CPNT_AMTRL3 == TRUE)
    UI32_T  fib_id = SYS_ADPT_DEFAULT_FIB;

    if (AMTRL3_PMGR_ClearAllDynamicARP(AMTRL3_TYPE_FLAGS_IPV4, fib_id) != TRUE)
        return NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY;
    else
        return NETCFG_TYPE_OK;
#else
#if 0
    if(IPAL_PMGR_DeleteAllDynamicIpNetToMediaEntry() != TRUE)
        return NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY;
    else
        return NETCFG_TYPE_OK;
#else
        printf("%s,%d\n",__FUNCTION__,__LINE__);
        return NETCFG_TYPE_OK;
#endif
#endif
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_GetNextIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Look up next ARP entry.
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
UI32_T ND_MGR_GetNextIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry)
{
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

#if (SYS_CPNT_AMTRL3 == TRUE)
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    AMTRL3_TYPE_ipNetToPhysicalEntry_T amtrl3_entry;

    memset(&amtrl3_entry, 0, sizeof(AMTRL3_TYPE_ipNetToPhysicalEntry_T));

    /*Lin.Li,for EPR:ES4827G-FLF-ZZ-00258*/
    amtrl3_entry.ip_net_to_physical_if_index = entry->ip_net_to_media_if_index;
    amtrl3_entry.ip_net_to_physical_type = (I32_T)entry->ip_net_to_media_type;
    amtrl3_entry.ip_net_to_physical_phys_address.phy_address_type = entry->ip_net_to_media_phys_address.phy_address_type;
    amtrl3_entry.ip_net_to_physical_phys_address.phy_address_len = entry->ip_net_to_media_phys_address.phy_address_len;
    amtrl3_entry.ip_net_to_physical_net_address.type = VAL_InetAddressType_ipv4;
    memcpy(amtrl3_entry.ip_net_to_physical_net_address.addr,
        &entry->ip_net_to_media_net_address,
        SYS_ADPT_IPV4_ADDR_LEN);
    memcpy(amtrl3_entry.ip_net_to_physical_phys_address.phy_address_octet_string,
            entry->ip_net_to_media_phys_address.phy_address_cctet_string,
            SYS_ADPT_MAC_ADDR_LEN);
    /*Lin.Li,for EPR:ES4827G-FLF-ZZ-00258 end*/

    if (AMTRL3_POM_GetNextIpNetToPhysicalEntry(AMTRL3_TYPE_FLAGS_IPV4,fib_id,&amtrl3_entry) != TRUE)
    {
        return NETCFG_TYPE_CAN_NOT_GET;
    }
    else
    {
        entry->ip_net_to_media_if_index = amtrl3_entry.ip_net_to_physical_if_index;
        memcpy(&entry->ip_net_to_media_net_address, amtrl3_entry.ip_net_to_physical_net_address.addr,sizeof(UI32_T));
        entry->ip_net_to_media_type = (I32_T)(amtrl3_entry.ip_net_to_physical_type);
        entry->ip_net_to_media_phys_address.phy_address_type = (UI32_T)(amtrl3_entry.ip_net_to_physical_phys_address.phy_address_type);
        entry->ip_net_to_media_phys_address.phy_address_len = (UI32_T)(amtrl3_entry.ip_net_to_physical_phys_address.phy_address_len);
        memcpy(entry->ip_net_to_media_phys_address.phy_address_cctet_string, amtrl3_entry.ip_net_to_physical_phys_address.phy_address_octet_string,NETCFG_TYPE_PHY_ADDRESEE_LENGTH);
        return NETCFG_TYPE_OK;
    }
#else
#if 0
    if(IPAL_PMGR_GetNextIpNetToMediaEntry() != TRUE)
        return NETCFG_TYPE_CAN_NOT_GET;
    else
        return NETCFG_TYPE_OK;
#else
        printf("%s,%d\n",__FUNCTION__,__LINE__);
        return NETCFG_TYPE_OK;
#endif
#endif
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_GetStatistic
 *-----------------------------------------------------------------------------
 * PURPOSE : Get ARP packet statistics.
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
UI32_T ND_MGR_GetStatistic(NETCFG_TYPE_IpNetToMedia_Statistics_T *stat)
{
    IPAL_Ipv4ArpStatistics_T arp_stat;

    if(stat == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&arp_stat, 0, sizeof (IPAL_Ipv4ArpStatistics_T));
    IPAL_NEIGH_GetIpv4ArpStatistics(&arp_stat);

    stat->in_request = arp_stat.in_request;
    stat->in_reply = arp_stat.in_reply;
    stat->out_request = arp_stat.out_request;
    stat->out_reply = arp_stat.out_reply;

    return NETCFG_TYPE_OK;
}


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_DeleteStaticIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete a static IPV6 neighbor entry.
 *
 * INPUT   : vid_ifindex, ip_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_DeleteStaticIpNetToPhysicalEntry(UI32_T vid_ifIndex, L_INET_AddrIp_T* ip_addr)
{
#if (SYS_CPNT_AMTRL3 == TRUE)
    UI32_T  fib_id = SYS_ADPT_DEFAULT_FIB;
    AMTRL3_TYPE_InetHostRouteEntry_T amtrl3_entry;
    UI8_T addrlen,al3_actionflag;

    memset(&amtrl3_entry, 0, sizeof(amtrl3_entry));

    if (ip_addr->type == L_INET_ADDR_TYPE_IPV4 ||
        ip_addr->type == L_INET_ADDR_TYPE_IPV4Z)
    {
        addrlen = SYS_ADPT_IPV4_ADDR_LEN;
        al3_actionflag = AMTRL3_TYPE_FLAGS_IPV4;
    }
    else if (ip_addr->type == L_INET_ADDR_TYPE_IPV6||
             ip_addr->type == L_INET_ADDR_TYPE_IPV6Z)
    {
        addrlen = SYS_ADPT_IPV6_ADDR_LEN;
        al3_actionflag = AMTRL3_TYPE_FLAGS_IPV6;
    }
    else
    {
       return NETCFG_TYPE_FAIL;
    }
    amtrl3_entry.dst_vid_ifindex = vid_ifIndex;
    amtrl3_entry.dst_inet_addr = *ip_addr;
    if (AMTRL3_PMGR_DeleteHostRoute(al3_actionflag, fib_id, &amtrl3_entry) != TRUE)
    {
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }
#endif /* SYS_CPNT_AMTRL3 */

#if (SYS_CPNT_IPAL == TRUE)
    if(IPAL_RESULT_OK != IPAL_NEIGH_DeleteNeighbor(vid_ifIndex, ip_addr))
    {
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }
#endif /* SYS_CPNT_IPAL */

/* if both AMTRL3 and IPAL are not enabled, return fail
 */
#if ((SYS_CPNT_AMTRL3 == TRUE) || (SYS_CPNT_IPAL == TRUE))
    return NETCFG_TYPE_OK;
#else
    return NETCFG_TYPE_FAIL;
#endif
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_DeleteAllDynamicIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic ARP entries.
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
UI32_T ND_MGR_DeleteAllDynamicIpNetToPhysicalEntry(UI32_T actionflags)
{
    UI32_T amtrl3_flags = 0;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;

#if (SYS_CPNT_AMTRL3 == TRUE)
    if (actionflags & ND_MGR_GET_FLAGS_IPV4)
        amtrl3_flags |=AMTRL3_TYPE_FLAGS_IPV4;

#if (SYS_CPNT_IPV6 == TRUE)
    if(actionflags & ND_MGR_GET_FLAGS_IPV6)
        amtrl3_flags |=AMTRL3_TYPE_FLAGS_IPV6;
#endif

    if (AMTRL3_PMGR_ClearAllDynamicARP(amtrl3_flags, fib_id) != TRUE)
    {
        DBGprintf("Fail to delete all dynamic neighbors from AMTRL3");
        return NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY;
    }
#endif /* #if (SYS_CPNT_AMTRL3 == TRUE) */

#if (SYS_CPNT_IPAL == TRUE)
    if (actionflags & ND_MGR_GET_FLAGS_IPV4)
    {
        if(IPAL_NEIGH_FlushAllDynamicNeighbor(IPAL_NEIGHBOR_ADDR_TYPE_IPV4) != IPAL_RESULT_OK)
        {
            DBGprintf("Fail to delete all IPv4 dynamic neighbors from TCP/IP ");
            return NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY;
        }
    }

#if (SYS_CPNT_IPV6 == TRUE)
    if (actionflags & ND_MGR_GET_FLAGS_IPV6)
    {
        if(IPAL_NEIGH_FlushAllDynamicNeighbor(IPAL_NEIGHBOR_ADDR_TYPE_IPV6) != IPAL_RESULT_OK)
        {
            DBGprintf("Fail to delete all IPv6 dynamic neighbors from TCP/IP ");
            return NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY;
        }
    }
#endif
#endif /* #if (SYS_CPNT_IPAL == TRUE) */

    return  NETCFG_TYPE_OK;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_GetNextIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Look up next IPV6 neighbor entry.
 *
 * INPUT   : NETCFG_TYPE_IpNetToMediaEntry_T *entry. key is _net_address
 *              possible action_flags are ND_MGR_GET_FLAGS_IPV4 and ND_MGR_GET_FLAGS_IPV6
 * OUTPUT  : NETCFG_TYPE_IpNetToMediaEntry_T *entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_GetNextIpNetToPhysicalEntry(UI32_T action_flags, NETCFG_TYPE_IpNetToPhysicalEntry_T *entry)
{
#if (SYS_CPNT_AMTRL3 == TRUE)
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    AMTRL3_TYPE_ipNetToPhysicalEntry_T amtrl3_entry;
    UI32_T al3_actionflag=0;
#endif

    if(entry == NULL)
    {
        DBGprintf("NULL input!");
        return NETCFG_TYPE_INVALID_ARG;
    }

#if (SYS_CPNT_AMTRL3 == TRUE)
    if (action_flags == ND_MGR_GET_FLAGS_IPV4)
    {
        memset(&amtrl3_entry, 0, sizeof(amtrl3_entry));
        amtrl3_entry.ip_net_to_physical_if_index = entry->ip_net_to_physical_if_index;
        amtrl3_entry.ip_net_to_physical_net_address = entry->ip_net_to_physical_net_address;
        amtrl3_entry.ip_net_to_physical_phys_address.phy_address_type = entry->ip_net_to_physical_phys_address.phy_address_type;
        amtrl3_entry.ip_net_to_physical_phys_address.phy_address_len = entry->ip_net_to_physical_phys_address.phy_address_len;
        memcpy(amtrl3_entry.ip_net_to_physical_phys_address.phy_address_octet_string,entry->ip_net_to_physical_phys_address.phy_address_cctet_string,SYS_ADPT_MAC_ADDR_LEN);

        if(amtrl3_entry.ip_net_to_physical_net_address.type==0)
        {
            if(action_flags == ND_MGR_GET_FLAGS_IPV4)
            {
                amtrl3_entry.ip_net_to_physical_net_address.type=L_INET_ADDR_TYPE_IPV4;
                amtrl3_entry.ip_net_to_physical_net_address.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            }
            /*else if (action_flags == ND_MGR_GET_FLAGS_IPV6)
            {
                amtrl3_entry.ip_net_to_physical_net_address.type =L_INET_ADDR_TYPE_IPV6;
                amtrl3_entry.ip_net_to_physical_net_address.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
            }*/
        }

        if(action_flags == ND_MGR_GET_FLAGS_IPV4)
            al3_actionflag = AMTRL3_TYPE_FLAGS_IPV4;
        /*else if (action_flags == ND_MGR_GET_FLAGS_IPV6)
            al3_actionflag = AMTRL3_TYPE_FLAGS_IPV6; */

        if(AMTRL3_POM_GetNextIpNetToPhysicalEntry(al3_actionflag, fib_id,&amtrl3_entry)!= TRUE)
        {
            return NETCFG_TYPE_CAN_NOT_GET;
        }

        entry->ip_net_to_physical_if_index = amtrl3_entry.ip_net_to_physical_if_index;
        entry->ip_net_to_physical_net_address = amtrl3_entry.ip_net_to_physical_net_address;

        entry->ip_net_to_physical_type = (int)(amtrl3_entry.ip_net_to_physical_type);
        entry->ip_net_to_physical_phys_address.phy_address_type = (UI32_T)(amtrl3_entry.ip_net_to_physical_phys_address.phy_address_type);
        entry->ip_net_to_physical_phys_address.phy_address_len = (UI32_T)(amtrl3_entry.ip_net_to_physical_phys_address.phy_address_len);
        memcpy(entry->ip_net_to_physical_phys_address.phy_address_cctet_string, amtrl3_entry.ip_net_to_physical_phys_address.phy_address_octet_string,entry->ip_net_to_physical_phys_address.phy_address_len);
        return NETCFG_TYPE_OK;
    }
#endif /* SYS_CPNT_AMTRL3 */


/* action_flags == ND_MGR_GET_FLAGS_IPV6, use IPAL */

#if (SYS_CPNT_IPAL == TRUE)
{
    IPAL_NeighborEntry_T ipal_neighbor;
    L_INET_AddrIp_T search_address;
    UI32_T addr_len;
    UI8_T  zero_ip[SYS_ADPT_IPV6_ADDR_LEN] = {0};
    UI32_T search_ifindex;
    UI32_T count;
    UI8_T  key[ND_MGR_HOST_ROUTE_CACHE_KEY_LEN];
    BOOL_T is_need_refresh_cache = FALSE;
    UI32_T i;

    /* Check whether need to refresh cache
     */
    if (action_flags == ND_MGR_GET_FLAGS_IPV4)
        addr_len = SYS_ADPT_IPV4_ADDR_LEN;
    else
        addr_len = SYS_ADPT_IPV6_ADDR_LEN;

    if (0 == memcmp(entry->ip_net_to_physical_net_address.addr, zero_ip, addr_len) ||
        nd_mgr_host_route_cache_last_upate > SYSFUN_GetSysTick() ||
        (SYSFUN_GetSysTick() - nd_mgr_host_route_cache_last_upate) > ND_MGR_HOST_ROUTE_CACHE_REFRESH_TIME ||
        entry->ip_net_to_physical_if_index < nd_mgr_host_route_cache_last_key.ip_net_to_physical_if_index ||
        0 > memcmp(&entry->ip_net_to_physical_net_address, &nd_mgr_host_route_cache_last_key.ip_net_to_physical_net_address, sizeof(L_INET_AddrIp_T)))
    {
        is_need_refresh_cache = TRUE;
    }
    else
    {
        memset(key, 0, sizeof(key));
        memcpy(key, (UI8_T*)&entry->ip_net_to_physical_if_index, ND_MGR_SIZEOF_HOST_ROUTE_IFINDEX);
        memcpy(key+ND_MGR_SIZEOF_HOST_ROUTE_IFINDEX, (UI8_T*)&entry->ip_net_to_physical_net_address, ND_MGR_SIZEOF_HOST_ROUTE_IPADDR);
        if (L_HISAM_GetNextRecord(&nd_mgr_host_route_cache_hisam_desc, ND_MGR_HOST_ROUTE_CACHE_KIDX, key, &ipal_neighbor))
        {
            goto get_success_and_return;
        }

        is_need_refresh_cache = TRUE;
    }

    if (is_need_refresh_cache)
    {
        L_HISAM_DeleteAllRecord(&nd_mgr_host_route_cache_hisam_desc);

        search_ifindex = entry->ip_net_to_physical_if_index;
        search_address = entry->ip_net_to_physical_net_address;
        count          = ND_MGR_HOST_ROUTE_CACHE_NBR;

        if (IPAL_RESULT_OK == IPAL_NEIGH_GetNextNNeighbor(search_ifindex,
                                                          &search_address,
                                                          &count,
                                                          nd_mgr_host_route_cache_ipal_neighbor))
        {
            for (i=0; i<count; i++)
            {
                if (nd_mgr_host_route_cache_ipal_neighbor[i].state & IPAL_NEIGH_NUD_NOARP) /* skip noarp entry */
                    continue;

                L_HISAM_SetRecord(&nd_mgr_host_route_cache_hisam_desc, &nd_mgr_host_route_cache_ipal_neighbor[i], FALSE);
            }
        }

        nd_mgr_host_route_cache_last_upate = SYSFUN_GetSysTick();
        nd_mgr_host_route_cache_last_key   = *entry;
    }

    memset(key, 0, sizeof(key));
    memcpy(key, (UI8_T*)&entry->ip_net_to_physical_if_index, ND_MGR_SIZEOF_HOST_ROUTE_IFINDEX);
    memcpy(key+ND_MGR_SIZEOF_HOST_ROUTE_IFINDEX, (UI8_T*)&entry->ip_net_to_physical_net_address, ND_MGR_SIZEOF_HOST_ROUTE_IPADDR);

    if (L_HISAM_GetNextRecord(&nd_mgr_host_route_cache_hisam_desc, ND_MGR_HOST_ROUTE_CACHE_KIDX, key, &ipal_neighbor))
    {
        goto get_success_and_return;
    }

    /* Clear cache after get last entry
     */
    L_HISAM_DeleteAllRecord(&nd_mgr_host_route_cache_hisam_desc);
    nd_mgr_host_route_cache_last_upate = 0;

    return NETCFG_TYPE_CAN_NOT_GET;

get_success_and_return:
    entry->ip_net_to_physical_if_index    = ipal_neighbor.ifindex;
    entry->ip_net_to_physical_net_address = ipal_neighbor.ip_addr;
    entry->ip_net_to_physical_last_update = ipal_neighbor.last_update;
    entry->ip_net_to_physical_phys_address.phy_address_type = SN_LAN;
    entry->ip_net_to_physical_phys_address.phy_address_len  = ipal_neighbor.phy_address_len;
    memcpy(entry->ip_net_to_physical_phys_address.phy_address_cctet_string, ipal_neighbor.phy_address, ipal_neighbor.phy_address_len);

    if (ipal_neighbor.state & IPAL_NEIGH_NUD_PERMANENT)
        entry->ip_net_to_physical_type = VAL_ipNetToMediaType_static;
    else
        entry->ip_net_to_physical_type = VAL_ipNetToMediaType_dynamic;

    if (ipal_neighbor.state & IPAL_NEIGH_NUD_REACHABLE)
        entry->ip_net_to_physical_state = NETCFG_TYPE_ND_NEIGHBOR_STATE_REACHABLE;
    else if (ipal_neighbor.state & IPAL_NEIGH_NUD_STALE)
        entry->ip_net_to_physical_state = NETCFG_TYPE_ND_NEIGHBOR_STATE_STALE;
    else if (ipal_neighbor.state & IPAL_NEIGH_NUD_DELAY)
        entry->ip_net_to_physical_state = NETCFG_TYPE_ND_NEIGHBOR_STATE_DELAY;
    else if (ipal_neighbor.state & IPAL_NEIGH_NUD_PROBE)
        entry->ip_net_to_physical_state = NETCFG_TYPE_ND_NEIGHBOR_STATE_PROBE;
    else
        entry->ip_net_to_physical_state = NETCFG_TYPE_ND_NEIGHBOR_STATE_UNKNOWN;

    return NETCFG_TYPE_OK;
}
#endif /* #if (SYS_CPNT_IPAL == TRUE) */

    return NETCFG_TYPE_FAIL;
}

#if (SYS_CPNT_IPV6 == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetNsDadAttempts
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the number of consecutive neighbor solicitation messages
 *                  that are sent on an interface while duplicate address detected
 *
 * INPUT   :
 *                  UI32_T  value: retransmission value
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_SetNsDadAttempts(UI32_T vid_ifindex, UI32_T value)
{
    //set to Linux kernel, need IPAL api
#if (SYS_CPNT_IPAL == TRUE)
    UI16_T l3_if_flags = 0;

    if((IPAL_RESULT_OK == IPAL_IF_GetIfFlags(vid_ifindex, &l3_if_flags)) && (l3_if_flags & IFF_UP))
    {
        if(IPAL_RESULT_OK != IPAL_ICMP_SetIpv6DadTransmits(vid_ifindex, value))
            return NETCFG_TYPE_FAIL;
    }
    return NETCFG_TYPE_OK;
#else
    return NETCFG_TYPE_FAIL;
#endif /*SYS_CPNT_IPAL*/

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetNsInterval
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the interval between IPv6 neighbor solicitation retransmissions
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T  milliseconds: ns retransmission interval in milliseconds
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_SetNsInterval(UI32_T vid_ifindex, UI32_T milliseconds)
{
#if (SYS_CPNT_IPAL == TRUE)
    UI16_T l3_if_flags = 0;

    // if 0, set to NSM; if not zero, set to Linux kernel and NSM
    if(milliseconds)
    {
        if((IPAL_RESULT_OK == IPAL_IF_GetIfFlags(vid_ifindex, &l3_if_flags)) && (l3_if_flags & IFF_UP))
        {
            if(IPAL_RESULT_OK !=IPAL_ICMP_SetIpv6NeighRetransTime(vid_ifindex,milliseconds))
                return NETCFG_TYPE_FAIL;
        }
    }
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    if(NSM_TYPE_RESULT_OK != NSM_PMGR_SetNsInterval(vid_ifindex,milliseconds))
        return NETCFG_TYPE_FAIL;
#endif

    return NETCFG_TYPE_OK;
#else
    return NETCFG_TYPE_FAIL;
#endif /* SYS_CPNT_IPAL */
}

UI32_T ND_MGR_UnsetNsInterval(UI32_T vid_ifindex)
{
#if (SYS_CPNT_IPAL == TRUE)
    UI16_T l3_if_flags = 0;

    if((IPAL_RESULT_OK == IPAL_IF_GetIfFlags(vid_ifindex, &l3_if_flags)) && (l3_if_flags & IFF_UP))
    {
        if(IPAL_RESULT_OK !=IPAL_ICMP_SetIpv6NeighRetransTime(vid_ifindex,SYS_DFLT_ND_NEIGHBOR_SOLICITATION_RETRANSMISSIONS_INTERVAL))
        {
            DBGprintf("fail to call IPAL_ICMP_SetIpv6NeighRetransTime(%ld,%d)", (long)vid_ifindex,SYS_DFLT_ND_NEIGHBOR_SOLICITATION_RETRANSMISSIONS_INTERVAL);
            return NETCFG_TYPE_FAIL;
        }
    }

#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    if(NSM_TYPE_RESULT_OK != NSM_PMGR_UnsetNsInterval(vid_ifindex))
    {
        DBGprintf("Fail to call NSM_PMGR_UnsetRsInterval(%ld)", (long)vid_ifindex);
        return NETCFG_TYPE_FAIL;
    }
#endif

    return NETCFG_TYPE_OK;
#else
    return NETCFG_TYPE_FAIL;
#endif /* SYS_CPNT_IPAL */
}

UI32_T ND_MGR_SetNdReachableTime(UI32_T vid_ifindex, UI32_T milliseconds)
{
#if (SYS_CPNT_IPAL == TRUE)
    UI16_T l3_if_flags = 0;
    if(milliseconds)
    {
        if((IPAL_RESULT_OK == IPAL_IF_GetIfFlags(vid_ifindex, &l3_if_flags)) && (l3_if_flags & IFF_UP))
        {
            if(IPAL_RESULT_OK != IPAL_ICMP_SetIpv6NeighReachableTime(vid_ifindex,milliseconds))
                return NETCFG_TYPE_FAIL;
        }
    }
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    if(NSM_TYPE_RESULT_OK != NSM_PMGR_SetRaReachableTime(vid_ifindex,milliseconds))
        return NETCFG_TYPE_FAIL;
#endif
    return NETCFG_TYPE_OK;
#else
    return NETCFG_TYPE_FAIL;
#endif /* SYS_CPNT_IPAL */
}

UI32_T ND_MGR_UnsetNdReachableTime(UI32_T vid_ifindex)
{
#if (SYS_CPNT_IPAL == TRUE)
    UI16_T l3_if_flags = 0;

    if((IPAL_RESULT_OK == IPAL_IF_GetIfFlags(vid_ifindex, &l3_if_flags)) && (l3_if_flags & IFF_UP))
    {
        if(IPAL_RESULT_OK != IPAL_ICMP_SetIpv6NeighReachableTime(vid_ifindex,SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_REACHABLE_TIME))
                return NETCFG_TYPE_FAIL;
    }
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    if(NSM_TYPE_RESULT_OK != NSM_PMGR_UnsetRaReachableTime(vid_ifindex))
        return NETCFG_TYPE_FAIL;
#endif
    return NETCFG_TYPE_OK;
#else
    return NETCFG_TYPE_FAIL;
#endif /* SYS_CPNT_IPAL */
}

#if (SYS_CPNT_IPV6_ROUTING == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRaLifetime
 *-----------------------------------------------------------------------------
 * PURPOSE : To configure the router lifetime value in IPv6 router advertisements on an interface,
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T  seconds: The validity of this router as a default router on this interface (in seconds).
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_SetRaLifetime(UI32_T vid_ifindex, UI32_T seconds)
{
    if(NSM_TYPE_RESULT_OK != NSM_PMGR_SetRaLifetime(vid_ifindex, seconds))
        return NETCFG_TYPE_FAIL;

    return NETCFG_TYPE_OK;
}
UI32_T ND_MGR_UnsetRaLifetime(UI32_T vid_ifindex)
{
    if(NSM_TYPE_RESULT_OK != NSM_PMGR_UnsetRaLifetime(vid_ifindex))
        return NETCFG_TYPE_FAIL;

    return NETCFG_TYPE_OK;
}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRaRouterPreference
 *-----------------------------------------------------------------------------
 * PURPOSE : configure a default router preference (DRP)
 *
 * INPUT   :        UI32_T vid_ifindex: vlan ifindex
 *                      UI32_T preference: on of
                            NETCFG_TYPE_ND_ROUTER_PERFERENCE_HIGH = 1,
                            NETCFG_TYPE_ND_ROUTER_PERFERENCE_MEDIUM=0,
                            NETCFG_TYPE_ND_ROUTER_PERFERENCE_LOW=3
 *
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_SetRaRouterPreference(UI32_T vid_ifindex, UI32_T  preference)
{
    if(NSM_TYPE_RESULT_OK != NSM_PMGR_SetRaRouterPreference(vid_ifindex,preference))
        return NETCFG_TYPE_FAIL;

    return NETCFG_TYPE_OK;
}

static UI32_T ND_MGR_LocalSetHopLimit(UI32_T value)
{
#if (SYS_CPNT_IPAL == TRUE)
    NETCFG_TYPE_L3_Interface_T  intf;
    UI16_T                      l3_if_flags = 0;

    //step1: set default value for IPAL and NSM
    if (value) /* non-zero */
    {
        if (IPAL_RESULT_OK != IPAL_IF_SetDefaultHopLimit(value))
            return NETCFG_TYPE_FAIL;
    }

    if (NSM_TYPE_RESULT_OK != NSM_PMGR_SetDefaultRaHopLimit(value))
        return NETCFG_TYPE_FAIL;

    //step2: set existing vlan
    memset(&intf, 0, sizeof(intf));
    while (NETCFG_OM_IP_GetNextL3Interface(&intf) == NETCFG_TYPE_OK)
    {
        if (value) /* non-zero */
        {
            if (  (IPAL_RESULT_OK == IPAL_IF_GetIfFlags(intf.ifindex, &l3_if_flags))
                &&(l3_if_flags & IFF_UP))
            {
                if (IPAL_RESULT_OK != IPAL_IF_SetIfHopLimit(intf.ifindex, value))
                    return NETCFG_TYPE_FAIL;
            }
        }
        NSM_PMGR_SetRaHopLimit(intf.ifindex, value);
    }
    return NETCFG_TYPE_OK;
#else
    return NETCFG_TYPE_FAIL;
#endif /* SYS_CPNT_IPAL */
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetHopLimit
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the maximum number of hops used in router advertisements
 *
 * INPUT   :    UI32_T value: hoplimit
 *
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_SetHopLimit(UI32_T value)
{
    return ND_MGR_LocalSetHopLimit(value);
}

UI32_T ND_MGR_UnsetHopLimit(void)
{
    return ND_MGR_LocalSetHopLimit(SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_HOPLIMIT);
}
#endif /* #if (SYS_CPNT_IPV6_ROUTING == TRUE) */
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

