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
#include "l_mm.h"
#include "arp_mgr.h"
#include "string.h"
#include "stdlib.h"
#include "l_stdlib.h"
#include "amtrl3_type.h"
#include "amtrl3_pom.h"
#include "amtrl3_pmgr.h"
#include "leaf_4001.h"

#include "l_prefix.h"
#include "ipal_types.h"
#include "ipal_arp_mgr.h"

/* FUNCTION NAME : ARP_MGR_InitiateProcessResources
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
void ARP_MGR_InitiateProcessResources(void)
{
    return;
}

/* FUNCTION NAME : ARP_MGR_GetIpNetToMediaEntry
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
UI32_T     ARP_MGR_GetIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry)
{
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
#if SYS_CPNT_AMTRL3
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    AMTRL3_TYPE_ipNetToPhysicalEntry_T amtrl3_entry;

    memset(&amtrl3_entry, 0, sizeof(AMTRL3_TYPE_ipNetToPhysicalEntry_T));
    amtrl3_entry.ip_net_to_physical_if_index = entry->ip_net_to_media_if_index;
    amtrl3_entry.ip_net_to_physical_net_address.address_type = VAL_InetAddressType_ipv4;
    memcpy(amtrl3_entry.ip_net_to_physical_net_address.addr.ipv4, &entry->ip_net_to_media_net_address, sizeof(UI8_T)*4);

    if(AMTRL3_POM_GetIpNetToPhysicalEntry(AMTRL3_TYPE_FLAGS_IPV4,fib_id,&amtrl3_entry)!= TRUE)
        return NETCFG_TYPE_CAN_NOT_GET;
    else
    {
        entry->ip_net_to_media_if_index = amtrl3_entry.ip_net_to_physical_if_index;
        memcpy(&entry->ip_net_to_media_net_address, amtrl3_entry.ip_net_to_physical_net_address.addr.ipv4,sizeof(UI32_T));
        entry->ip_net_to_media_type = (I32_T)(amtrl3_entry.ip_net_to_physical_type);
        entry->ip_net_to_media_phys_address.phy_address_type = (UI32_T)(amtrl3_entry.ip_net_to_physical_phys_address.phy_address_type);
        entry->ip_net_to_media_phys_address.phy_address_len = (UI32_T)(amtrl3_entry.ip_net_to_physical_phys_address.phy_address_len);
        memcpy(entry->ip_net_to_media_phys_address.phy_address_cctet_string, amtrl3_entry.ip_net_to_physical_phys_address.phy_address_octet_string,NETCFG_TYPE_PHY_ADDRESEE_LENGTH);
        return NETCFG_TYPE_OK;
    }
#else
#if 0
    if(IPAL_PMGR_GetIpNetToMediaEntry() != TRUE)
        return NETCFG_TYPE_CAN_NOT_GET;
#else
        printf("%s,%d\n",__FUNCTION__,__LINE__);
        return NETCFG_TYPE_OK;
#endif
#endif
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ARP_MGR_AddStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a static ARP entry.
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
UI32_T ARP_MGR_AddStaticIpNetToMediaEntry(UI32_T vid_ifIndex,
                                                            UI32_T ip_addr,
                                                            UI32_T phy_addr_len,
                                                            UI8_T *phy_addr)
{
    if(phy_addr == NULL)
        return NETCFG_TYPE_INVALID_ARG;

#if SYS_CPNT_AMTRL3
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    AMTRL3_TYPE_InetHostRouteEntry_T amtrl3_entry;
    UI32_T type = VAL_ipNetToPhysicalExtType_static;

    memset(&amtrl3_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));

    amtrl3_entry.dst_vid_ifindex = vid_ifIndex;
    amtrl3_entry.dst_inet_addr.address_type = VAL_InetAddressType_ipv4;
    memcpy(amtrl3_entry.dst_inet_addr.addr.ipv4, &ip_addr, sizeof(UI8_T)*4);
    memcpy(amtrl3_entry.dst_mac, phy_addr, phy_addr_len);
    amtrl3_entry.lport = SYS_ADPT_DESTINATION_PORT_UNKNOWN; /*add for static arp --xiongyu  20081031*/
    
    
    if (AMTRL3_PMGR_SetHostRoute(AMTRL3_TYPE_FLAGS_IPV4, fib_id, &amtrl3_entry, type) != TRUE)
        return NETCFG_TYPE_CAN_NOT_ADD;
    else
        return NETCFG_TYPE_OK;
#else
#if 0
    if(IPAL_PMGR_AddStaticIpNetToMediaEntry() != TRUE)
        return NETCFG_TYPE_CAN_NOT_ADD;
    else 
        return NETCFG_TYPE_OK;
#else
        printf("%s,%d\n",__FUNCTION__,__LINE__);
        return NETCFG_TYPE_OK;
#endif
#endif
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ARP_MGR_DeleteStaticIpNetToMediaEntry
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
UI32_T ARP_MGR_DeleteStaticIpNetToMediaEntry(UI32_T vid_ifIndex, UI32_T ip_addr)
{
#if SYS_CPNT_AMTRL3
    UI32_T      fib_id = SYS_ADPT_DEFAULT_FIB;
    AMTRL3_TYPE_InetHostRouteEntry_T amtrl3_entry;

    memset(&amtrl3_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));

    amtrl3_entry.dst_vid_ifindex = vid_ifIndex;
    amtrl3_entry.dst_inet_addr.address_type = VAL_InetAddressType_ipv4;
    memcpy(amtrl3_entry.dst_inet_addr.addr.ipv4, &ip_addr, sizeof(UI8_T)*4);
    
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
 * ROUTINE NAME : ARP_MGR_SetIpNetToMediaEntryTimeout
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
UI32_T ARP_MGR_SetIpNetToMediaEntryTimeout(UI32_T age_time, UI32_T pre_timeout)
{
#if SYS_CPNT_AMTRL3
    UI32_T  fib_id = SYS_ADPT_DEFAULT_FIB;

    if (AMTRL3_PMGR_SetIpNetToPhysicalEntryTimeout(AMTRL3_TYPE_FLAGS_IPV4, fib_id, age_time, 0) != TRUE)
        return NETCFG_TYPE_CAN_NOT_SET;
#endif

#if 0
    if(IPAL_PMGR_SetARPTimeout() != TRUE)
    {
#if SYS_CPNT_AMTRL3
        AMTRL3_PMGR_SetIpNetToPhysicalEntryTimeout(AMTRL3_TYPE_FLAGS_IPV4, fib_id, pre_timeout, 0);
#endif
        return NETCFG_TYPE_CAN_NOT_SET;
    }
#endif
    return NETCFG_TYPE_OK;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ARP_MGR_DeleteAllDynamicIpNetToMediaEntry
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
UI32_T ARP_MGR_DeleteAllDynamicIpNetToMediaEntry(void)
{
#if SYS_CPNT_AMTRL3
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
 * ROUTINE NAME : ARP_MGR_GetNextIpNetToMediaEntry
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
UI32_T ARP_MGR_GetNextIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry)
{
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

#if SYS_CPNT_AMTRL3
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    AMTRL3_TYPE_ipNetToPhysicalEntry_T amtrl3_entry;

    memset(&amtrl3_entry, 0, sizeof(AMTRL3_TYPE_ipNetToPhysicalEntry_T));

    /*Lin.Li,for EPR:ES4827G-FLF-ZZ-00258*/
    amtrl3_entry.ip_net_to_physical_if_index = entry->ip_net_to_media_if_index;
    amtrl3_entry.ip_net_to_physical_type = (I32_T)entry->ip_net_to_media_type;
    amtrl3_entry.ip_net_to_physical_phys_address.phy_address_type = entry->ip_net_to_media_phys_address.phy_address_type;
    amtrl3_entry.ip_net_to_physical_phys_address.phy_address_len = entry->ip_net_to_media_phys_address.phy_address_len;
    amtrl3_entry.ip_net_to_physical_net_address.address_type = VAL_InetAddressType_ipv4;
    memcpy(amtrl3_entry.ip_net_to_physical_net_address.addr.ipv4, 
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
        memcpy(&entry->ip_net_to_media_net_address, amtrl3_entry.ip_net_to_physical_net_address.addr.ipv4,sizeof(UI32_T));
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
 * ROUTINE NAME : ARP_MGR_GetStatistic
 *-----------------------------------------------------------------------------
 * PURPOSE : Get ARP packet statistics.
 *
 * INPUT   : None.
 *
 * OUTPUT  : stat.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *          NETCFG_TYPE_INVALID_ARG
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ARP_MGR_GetStatistic(NETCFG_TYPE_IpNetToMedia_Statistics_T *stat)
{
    IPAL_IPv4_ARP_Statistics_T arp_stat;
    
    if(stat == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&arp_stat, 0, sizeof (IPAL_IPv4_ARP_Statistics_T));
    IPAL_ARP_MGR_Statistics(&arp_stat);

    stat->in_request = arp_stat.in_request;
    stat->in_reply = arp_stat.in_reply;
    stat->out_request = arp_stat.out_request;
    stat->out_reply = arp_stat.out_reply;
    
    return NETCFG_TYPE_OK;
}


