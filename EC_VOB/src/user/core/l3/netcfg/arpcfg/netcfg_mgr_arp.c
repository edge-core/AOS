/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_MGR_ARP.C
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
#define     BACKDOOR_OPEN
#include "sysfun.h"
#include "sys_type.h"
#include "ip_lib.h"
#include "netcfg_type.h"
#include "netcfg_mgr_arp.h"
#include "leaf_1213.h"
#include "leaf_es3626a.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "netcfg_om_arp.h"
#include "netcfg_om_ip.h"
#ifdef  BACKDOOR_OPEN
#include "backdoor_mgr.h"
#endif
#include "l_threadgrp.h"
#include "string.h"
#include "stdlib.h"
#include "l_inet.h"
#include "netcfg_proc_comm.h"
#include "arp_mgr.h"
SYSFUN_DECLARE_CSC

static UI32_T arpcfg_static_arp_num = 0;
static BOOL_T is_provision_complete = FALSE;

#define SN_LAN                          1
#define PREFIXLEN2MASK(len)             ~((1 << (32 - len)) - 1)

static int NETCFG_MGR_ARP_Backdoor_GetIP(UI32_T * IPaddr);
static int NETCFG_MGR_ARP_Backdoor_AtoIP(UI8_T *s, UI8_T *ip);
static void NETCFG_MGR_ARP_Backdoor_SignalRifCreate(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ARP_Backdoor_SignalRifActive(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ARP_Backdoor_SignalRifDown(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ARP_Backdoor_SignalRifDestroy(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ARP_Backdoor_GetStaticEntry(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ARP_Backdoor_GetAllStaticEntry(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ARP_Backdoor_DeleteAllStaticEntry(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ARP_Backdoor_Main(void);
static BOOL_T NETCFG_MGR_ARP_GetARPEntryType(UI32_T ip_addr, UI32_T *entry_type);
static BOOL_T   NETCFG_MGR_ARP_CheckPhysicalAddress(UI8_T *phy_addr);


static int NETCFG_MGR_ARP_Backdoor_AtoIP(UI8_T *s, UI8_T *ip)
{
        UI8_T token[20];
        int   i,j;  /* i for s[]; j for token[] */
        int   k;    /* k for ip[] */
    
        UI8_T temp[4];
    
        i = 0;
        j = 0;
        k = 0;
    
        while (s[i] != '\0')
        {
            if (s[i] == '.')
            {
                token[j] = '\0';
                if (strlen((char *)token) < 1 || strlen((char *)token) > 3 ||
                    atoi((char *)token) < 0 || atoi((char *)token) > 255)
                {
                    return 0;
                }
                else if (k >= 4)
                {
                    return 0;
                }
                else
                {
                    temp[k++] =(UI8_T)atoi((char *)token);
                    i++; j = 0;
                }
            }
            else if (!(s[i] >= '0' && s[i] <= '9'))
            {
                return 0;
            }
            else
            {
                token[j++] = s[i++];
            }
    
        } /* while */
    
        token[j] = '\0';
        if (strlen((char*)token) < 1 || strlen((char*)token) > 3 ||
            atoi((char *)token) < 0 || atoi((char *)token) > 255)
        {
            return 0;
        }
        else if (k != 3)
        {
            return 0;
        }
    
        temp[k]=(UI8_T)atoi((char *)token);
        
        ip[0] = temp[0];
        ip[1] = temp[1];
        ip[2] = temp[2];
        ip[3] = temp[3];
    
        return 1;

}
static int NETCFG_MGR_ARP_Backdoor_GetIP(UI32_T * IPaddr)
{
    int ret = 0;
    UI8_T   *temp_ip = (UI8_T *)IPaddr;
    UI8_T   buffer[20] = {0};

    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    ret = NETCFG_MGR_ARP_Backdoor_AtoIP(buffer, temp_ip);

    return  ret;
}

/* FUNCTION NAME : NETCFG_MGR_ARP_Backdoor_SignalRifCreate
 * PURPOSE:Backdoor of when a rif Create signal ARP
 *
 * INPUT:tg_handle,member_id
 *    
 *
 * OUTPUT:None
 *    
 *
 * RETURN:None
 *  
 *
 * NOTES:
 *
 */
static void NETCFG_MGR_ARP_Backdoor_SignalRifCreate(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI32_T ip_addr;
    UI32_T ip_mask;
    UI32_T ret;
    
    BACKDOOR_MGR_Printf("\n\r Input IP address(x.x.x.x): ");
    if(!NETCFG_MGR_ARP_Backdoor_GetIP(&ip_addr))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP address\n");

    BACKDOOR_MGR_Printf("\n\r Input IP Mask(x.x.x.x): ");
    if(!NETCFG_MGR_ARP_Backdoor_GetIP(&ip_mask))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP mask\n");
    
    L_THREADGRP_Execution_Request(tg_handle, member_id);
    ret = NETCFG_MGR_ARP_SignalRifCreate(ip_addr, ip_mask);
    L_THREADGRP_Execution_Release(tg_handle, member_id);    
    switch(ret)
    {
        case NETCFG_TYPE_NOT_MASTER_MODE:
            BACKDOOR_MGR_Printf("\nNot in master mode\n");
            break;
        
        case NETCFG_TYPE_INVALID_ARG:
            BACKDOOR_MGR_Printf("\nInvalid IP address or IP mask\n");
            break;

        case NETCFG_TYPE_INVALID_IP:
            BACKDOOR_MGR_Printf("\nThe Rif not create\n");
            break;
            
        case NETCFG_TYPE_OK:
            BACKDOOR_MGR_Printf("\nOK!!!\n");
            break;

        default:
            BACKDOOR_MGR_Printf("\nFail\n");
            break;
    }
}

/* FUNCTION NAME : NETCFG_MGR_ARP_Backdoor_SignalRifActive
 * PURPOSE:Backdoor of when a exist rif active signal ARP
 *
 * INPUT:tg_handle,member_id
 *    
 *
 * OUTPUT:None
 *    
 *
 * RETURN:None
 *  
 *
 * NOTES:
 *
 */
static void NETCFG_MGR_ARP_Backdoor_SignalRifActive(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI32_T ip_addr;
    UI32_T ip_mask;
    UI32_T ret;
    
    BACKDOOR_MGR_Printf("\n\r Input IP address(x.x.x.x): ");
    if(!NETCFG_MGR_ARP_Backdoor_GetIP(&ip_addr))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP address\n");

    BACKDOOR_MGR_Printf("\n\r Input IP Mask(x.x.x.x): ");
    if(!NETCFG_MGR_ARP_Backdoor_GetIP(&ip_mask))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP mask\n");
    
    L_THREADGRP_Execution_Request(tg_handle, member_id);
    ret = NETCFG_MGR_ARP_SignalRifActive(ip_addr, ip_mask);
    L_THREADGRP_Execution_Release(tg_handle, member_id);    
    switch(ret)
    {
        case NETCFG_TYPE_NOT_MASTER_MODE:
            BACKDOOR_MGR_Printf("\nNot in master mode\n");
            break;
        
        case NETCFG_TYPE_INVALID_ARG:
            BACKDOOR_MGR_Printf("\nInvalid IP address or IP mask\n");
            break;

        case NETCFG_TYPE_INVALID_IP:
            BACKDOOR_MGR_Printf("\nThe Rif not active\n");
            break;
            
        case NETCFG_TYPE_OK:
            BACKDOOR_MGR_Printf("\nOK!!!\n");
            break;

        default:
            BACKDOOR_MGR_Printf("\nFail\n");
            break;
    }
}

/* FUNCTION NAME : NETCFG_MGR_ARP_Backdoor_SignalRifDown
 * PURPOSE:Backdoor of when a exist rif down signal ARP
 *
 * INPUT:tg_handle,member_id
 *    
 *
 * OUTPUT:None
 *    
 *
 * RETURN:None
 *  
 *
 * NOTES:
 *
 */
static void NETCFG_MGR_ARP_Backdoor_SignalRifDown(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI32_T ip_addr;
    UI32_T ip_mask;
    UI32_T ret;
    
    BACKDOOR_MGR_Printf("\n\r Input IP address(x.x.x.x): ");
    if(!NETCFG_MGR_ARP_Backdoor_GetIP(&ip_addr))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP address\n");

    BACKDOOR_MGR_Printf("\n\r Input IP Mask(x.x.x.x): ");
    if(!NETCFG_MGR_ARP_Backdoor_GetIP(&ip_mask))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP mask\n");
    
    L_THREADGRP_Execution_Request(tg_handle, member_id);
    ret = NETCFG_MGR_ARP_SignalRifDown(ip_addr, ip_mask);
    L_THREADGRP_Execution_Release(tg_handle, member_id);    
    switch(ret)
    {
        case NETCFG_TYPE_NOT_MASTER_MODE:
            BACKDOOR_MGR_Printf("\nNot in master mode\n");
            break;
        
        case NETCFG_TYPE_INVALID_ARG:
            BACKDOOR_MGR_Printf("\nInvalid IP address or IP mask\n");
            break;

        case NETCFG_TYPE_INVALID_IP:
            BACKDOOR_MGR_Printf("\nThe Rif not down\n");
            break;
            
        case NETCFG_TYPE_OK:
            BACKDOOR_MGR_Printf("\nOK!!!\n");
            break;

        default:
            BACKDOOR_MGR_Printf("\nFail\n");
            break;
    }
}

/* FUNCTION NAME : NETCFG_MGR_ARP_Backdoor_SignalRifDestroy
 * PURPOSE:Backdoor of when destroy a rif signal ARP
 *
 * INPUT:tg_handle,member_id
 *    
 *
 * OUTPUT:None
 *    
 *
 * RETURN:None
 *  
 *
 * NOTES:
 *
 */
static void NETCFG_MGR_ARP_Backdoor_SignalRifDestroy(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI32_T ip_addr;
    UI32_T ip_mask;
    UI32_T ret;
    
    BACKDOOR_MGR_Printf("\n\r Input Destroy IP address(x.x.x.x): ");
    if(!NETCFG_MGR_ARP_Backdoor_GetIP(&ip_addr))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP address\n");

    BACKDOOR_MGR_Printf("\n\r Input IP Mask(x.x.x.x): ");
    if(!NETCFG_MGR_ARP_Backdoor_GetIP(&ip_mask))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP mask\n");
    
    L_THREADGRP_Execution_Request(tg_handle, member_id);
    ret = NETCFG_MGR_ARP_SignalRifDestroy(ip_addr, ip_mask);
    L_THREADGRP_Execution_Release(tg_handle, member_id);    
    switch(ret)
    {
        case NETCFG_TYPE_NOT_MASTER_MODE:
            BACKDOOR_MGR_Printf("\nNot in master mode\n");
            break;
        
        case NETCFG_TYPE_INVALID_ARG:
            BACKDOOR_MGR_Printf("\nInvalid IP address or IP mask\n");
            break;
            
        case NETCFG_TYPE_OK:
            BACKDOOR_MGR_Printf("\nOK!!!\n");
            break;
        
        default:
            BACKDOOR_MGR_Printf("\nFail\n");
            break;
    }
}

/* FUNCTION NAME : NETCFG_MGR_ARP_Backdoor_GetStaticEntry
 * PURPOSE:
 *      Backdoor for get a static arp entry.
 *
 *
 * INPUT:tg_handle,member_id
 *    
 *
 * OUTPUT:None
 *    
 *
 * RETURN:None
 *  
 *
 * NOTES:
 *
 */
static void NETCFG_MGR_ARP_Backdoor_GetStaticEntry(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI32_T  ip_addr;
    UI8_T   ip_str[18] = {0};
    UI32_T  ifindex;
    char    *terminal;
    char    buf[16];
    BOOL_T  ret;
    NETCFG_TYPE_StaticIpNetToMediaEntry_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
    
    BACKDOOR_MGR_Printf("\n\r Input IP address(x.x.x.x): ");
    if(!NETCFG_MGR_ARP_Backdoor_GetIP(&ip_addr))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP address\n");

    BACKDOOR_MGR_Printf("\n\r Input Ifindex: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 15);
    ifindex = (UI32_T) strtoul(buf, &terminal, 10);
    BACKDOOR_MGR_Printf ("\n");

    entry.ip_net_to_media_entry.ip_net_to_media_if_index = ifindex;
    entry.ip_net_to_media_entry.ip_net_to_media_net_address = ip_addr;
    
    L_THREADGRP_Execution_Request(tg_handle, member_id);
    ret = NETCFG_OM_ARP_GetStaticEntry(&entry);
    L_THREADGRP_Execution_Release(tg_handle, member_id);   

    if(ret == TRUE)
    {
        L_INET_Ntoa(entry.ip_net_to_media_entry.ip_net_to_media_net_address,ip_str);
        BACKDOOR_MGR_Printf("\n\r The static ARP information: ");
        BACKDOOR_MGR_Printf("\n\r Ifindex: %d", entry.ip_net_to_media_entry.ip_net_to_media_if_index);
        BACKDOOR_MGR_Printf("\n\r IP address: %s", ip_str);
        BACKDOOR_MGR_Printf("\n\r Physical address: %02X-%02X-%02X-%02X-%02X-%02X", 
                             entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string[0],
                             entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string[1],
                             entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string[2],
                             entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string[3],
                             entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string[4],
                             entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string[5]);
        if(entry.status == TRUE)
            BACKDOOR_MGR_Printf("\n\r Status: TRUE");
        else
            BACKDOOR_MGR_Printf("\n\r Status: FALSE");
    }
    else
        BACKDOOR_MGR_Printf("\n\r Cannot find the static ARP entry.");
}


/* FUNCTION NAME : NETCFG_MGR_ARP_Backdoor_GetAllStaticEntry
 * PURPOSE:
 *      Backdoor for get all of static arp entries.
 *
 *
 * INPUT:tg_handle,member_id
 *    
 *
 * OUTPUT:None
 *    
 *
 * RETURN:None
 *  
 *
 * NOTES:
 *
 */
static void NETCFG_MGR_ARP_Backdoor_GetAllStaticEntry(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI8_T   ip_str[18] = {0};
    NETCFG_TYPE_StaticIpNetToMediaEntry_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));

    L_THREADGRP_Execution_Request(tg_handle, member_id);
    while(NETCFG_OM_ARP_GetNextStaticEntry(&entry) == TRUE)
    {
        BACKDOOR_MGR_Printf("\n\r");
        L_INET_Ntoa(entry.ip_net_to_media_entry.ip_net_to_media_net_address,ip_str);
        BACKDOOR_MGR_Printf("\n\r Ifindex: %d", entry.ip_net_to_media_entry.ip_net_to_media_if_index);
        BACKDOOR_MGR_Printf("\n\r IP address: %s", ip_str);
        BACKDOOR_MGR_Printf("\n\r Physical address: %02X-%02X-%02X-%02X-%02X-%02X", 
                             entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string[0],
                             entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string[1],
                             entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string[2],
                             entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string[3],
                             entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string[4],
                             entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string[5]);
        if(entry.status == TRUE)
            BACKDOOR_MGR_Printf("\n\r Status: TRUE");
        else
            BACKDOOR_MGR_Printf("\n\r Status: FALSE");
    }
    L_THREADGRP_Execution_Release(tg_handle, member_id);   
}

/* FUNCTION NAME : NETCFG_MGR_ARP_Backdoor_GetAllStaticEntry
 * PURPOSE:
 *      Backdoor for delete all of static arp entries.
 *
 *
 * INPUT:tg_handle,member_id
 *    
 *
 * OUTPUT:None
 *    
 *
 * RETURN:None
 *  
 *
 * NOTES:
 *
 */
static void NETCFG_MGR_ARP_Backdoor_DeleteAllStaticEntry(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    NETCFG_TYPE_StaticIpNetToMediaEntry_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));

    L_THREADGRP_Execution_Request(tg_handle, member_id);
    NETCFG_OM_ARP_DeleteAllStaticEntry();

    if(NETCFG_OM_ARP_GetNextStaticEntry(&entry) == TRUE)
        BACKDOOR_MGR_Printf("\n\r Cannot delete all static ARP entries");      
    else
        BACKDOOR_MGR_Printf("\n\r There isn't any static ARP entry");      
    L_THREADGRP_Execution_Release(tg_handle, member_id);
}

/* FUNCTION NAME : NETCFG_MGR_ARP_BackDoorMain
 * PURPOSE:Backdoor of ARP
 *
 * INPUT:None
 *    
 *
 * OUTPUT:None
 *    
 *
 * RETURN:None
 *  
 *
 * NOTES:
 *
 */
static void NETCFG_MGR_ARP_Backdoor_Main(void)
{
    int ch;
    BOOL_T eof = FALSE;
    char buf[16];
    char *terminal;
    L_THREADGRP_Handle_T tg_handle;
    UI32_T               backdoor_member_id;

    tg_handle      = NETCFG_PROC_COMM_GetNetcfgTGHandle();

    /* Join thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &backdoor_member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while (eof == FALSE)
    {
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("0.  Exit\n");
        BACKDOOR_MGR_Printf("1.  Signal a Rif Create\n");
        BACKDOOR_MGR_Printf("2.  Signal an exist Rif Active\n");
        BACKDOOR_MGR_Printf("3.  Signal an exist Rif Down\n");
        BACKDOOR_MGR_Printf("4.  Signal a Rif Destroy\n");
        BACKDOOR_MGR_Printf("5.  Get Static ARP Entry\n");
        BACKDOOR_MGR_Printf("6.  Get All Static ARP Entry\n");
        BACKDOOR_MGR_Printf("7.  Delete All Static ARP Entry\n");
        BACKDOOR_MGR_Printf("    select =");

        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int) strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf ("\n");

        switch(ch) 
        {
            case 0:
                eof = TRUE;
                break;
                
            case 1:
                NETCFG_MGR_ARP_Backdoor_SignalRifCreate(tg_handle, backdoor_member_id);
                break;
                
            case 2:
                NETCFG_MGR_ARP_Backdoor_SignalRifActive(tg_handle, backdoor_member_id);
                break;

            case 3:
                NETCFG_MGR_ARP_Backdoor_SignalRifDown(tg_handle, backdoor_member_id);
                break;
                
            case 4:
                NETCFG_MGR_ARP_Backdoor_SignalRifDestroy(tg_handle, backdoor_member_id);
                break;
                               
            case 5:
                NETCFG_MGR_ARP_Backdoor_GetStaticEntry(tg_handle, backdoor_member_id);
                break;
                
            case 6:
                NETCFG_MGR_ARP_Backdoor_GetAllStaticEntry(tg_handle, backdoor_member_id);
                break;
                
            case 7:
                NETCFG_MGR_ARP_Backdoor_DeleteAllStaticEntry(tg_handle, backdoor_member_id);
                break;
                
            default:
                ch = 0;
                break;
        }
    }    
    
    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
}

/* FUNCTION NAME : NETCFG_MGR_ARP_GetARPEntryType
 * PURPOSE:Get the ARP type of an ARP entry.
 *
 * INPUT:ip_addr
 *    
 *
 * OUTPUT:entry_type
 *    
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
static BOOL_T NETCFG_MGR_ARP_GetARPEntryType(UI32_T ip_addr, UI32_T *entry_type)
{
    NETCFG_TYPE_IpNetToMediaEntry_T             entry;
    NETCFG_TYPE_IPv4RifConfig_T                 rif_config;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_IpNetToMediaEntry_T));
    memset(&rif_config, 0, sizeof(NETCFG_TYPE_IPv4RifConfig_T));

    memcpy(rif_config.ip_addr, &ip_addr, sizeof(UI8_T)*SYS_ADPT_IPV4_ADDR_LEN);
    if(NETCFG_OM_IP_GetRifFromSubnet(&rif_config) != NETCFG_TYPE_OK)
        return FALSE;
       
    entry.ip_net_to_media_net_address = ip_addr;
    entry.ip_net_to_media_if_index = rif_config.ifindex;
    
    if(ARP_MGR_GetIpNetToMediaEntry(&entry) == NETCFG_TYPE_OK)
    {
        *entry_type = entry.ip_net_to_media_type;
        return TRUE;
    }
    else
        return FALSE;    
}

/* FUNCTION NAME : NETCFG_MGR_ARP_CheckPhysicalAddress
 * PURPOSE:check if ARP physical address valid.
 *
 * INPUT:phy_addr
 *    
 *
 * OUTPUT:None
 *    
 *
 * RETURN:
 *    TRUE  --  valid
 *    FALSE --  invalid
 *
 * NOTES:ARP physical address should not be all-zero, broastcast,multicast,eaps address.
 *
 */
 static BOOL_T   NETCFG_MGR_ARP_CheckPhysicalAddress(UI8_T *phy_addr)
{
    UI8_T null_addr[SYS_ADPT_MAC_ADDR_LEN] = {0};
    UI8_T broadcast_addr[SYS_ADPT_MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    UI8_T mutilcast_addr[SYS_ADPT_MAC_ADDR_LEN] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    UI8_T eaps_addr[SYS_ADPT_MAC_ADDR_LEN] = {0x00, 0xe0, 0x2b, 0x00, 0x00, 0x04};
    
    if (phy_addr == NULL)
        return FALSE;
    if(memcmp(phy_addr, null_addr, SYS_ADPT_MAC_ADDR_LEN) == 0 || 
       memcmp(phy_addr, broadcast_addr, SYS_ADPT_MAC_ADDR_LEN) == 0 ||
       memcmp(phy_addr, mutilcast_addr, 1) == 0 ||
       memcmp(phy_addr, eaps_addr, SYS_ADPT_MAC_ADDR_LEN) == 0 )
        return FALSE;
    else
        return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_ARP_InitiateProcessResources
 * PURPOSE:
 *      Initialize NETCFG_MGR_ARP used system resource, eg. protection semaphore.
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
BOOL_T NETCFG_MGR_ARP_InitiateProcessResources(void)
{
    NETCFG_OM_ARP_Init();
    ARP_MGR_InitiateProcessResources();
    return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_ARP_Create_InterCSC_Relation
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
void NETCFG_MGR_ARP_Create_InterCSC_Relation(void)
{
#ifdef  BACKDOOR_OPEN
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("arpcfg",
                                                      SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
                                                      NETCFG_MGR_ARP_Backdoor_Main);
#endif
} 

/* FUNCTION NAME : NETCFG_MGR_ARP_EnterMasterMode
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
void NETCFG_MGR_ARP_EnterMasterMode (void)
{
    NETCFG_OM_ARP_Init();
    SYSFUN_ENTER_MASTER_MODE();
} 

/* FUNCTION NAME : NETCFG_MGR_ARP_ProvisionComplete
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
void NETCFG_MGR_ARP_ProvisionComplete(void)
{
    is_provision_complete = TRUE;
} 

/* FUNCTION NAME : NETCFG_MGR_ARP_EnterSlaveMode
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
void NETCFG_MGR_ARP_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/* FUNCTION NAME : NETCFG_MGR_ARP_SetTransitionMode
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
void NETCFG_MGR_ARP_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

/* FUNCTION NAME : NETCFG_MGR_ARP_EnterTransitionMode
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
 /*Donny.li modify for ARP stacking.2008.08.07 */
void NETCFG_MGR_ARP_EnterTransitionMode (void)
{
    NETCFG_TYPE_StaticIpNetToMediaEntry_T entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
    
    SYSFUN_ENTER_TRANSITION_MODE();
    is_provision_complete = FALSE;
    NETCFG_OM_ARP_SetTimeout(SYS_DFLT_IP_NET_TO_MEDIA_ENTRY_TIMEOUT);
    while(NETCFG_OM_ARP_GetNextStaticEntry(&entry) == TRUE)
    {
        NETCFG_OM_ARP_DeleteStaticEntry(&entry);
    }
}   
/*Donny.li end modify for ARP stacking.2008.08.07 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ARP_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for ARP MGR.
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
BOOL_T NETCFG_MGR_ARP_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    NETCFG_MGR_ARP_IPCMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (NETCFG_MGR_ARP_IPCMsg_T*)msgbuf_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.result_bool = FALSE;
        msgbuf_p->msg_size = NETCFG_MGR_ARP_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    switch (msg_p->type.cmd)
    {
        case NETCFG_MGR_ARP_IPC_CREATESTATICARP:
            msg_p->type.result_ui32 =
                NETCFG_MGR_ARP_AddStaticIpNetToMediaEntry(msg_p->data.arg_grp1.arg1,
                                                        msg_p->data.arg_grp1.arg2,
                                                        msg_p->data.arg_grp1.arg3,
                                                        msg_p->data.arg_grp1.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_ARP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_ARP_IPC_DELETESTATICARP:
            msg_p->type.result_ui32 = NETCFG_MGR_ARP_DeleteStaticIpNetToMediaEntry(msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_ARP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_ARP_IPC_DELETEALLDYNAMIC:
            msg_p->type.result_ui32 = NETCFG_MGR_ARP_DeleteAllDynamicIpNetToMediaEntry();
            msgbuf_p->msg_size = NETCFG_MGR_ARP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_ARP_IPC_SETTIMEOUT:
            msg_p->type.result_ui32 = NETCFG_MGR_ARP_SetIpNetToMediaTimeout(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ARP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_ARP_IPC_GETNEXTARPENTRY:
            msg_p->type.result_ui32 = NETCFG_MGR_ARP_GetNextIpNetToMediaEntry(&(msg_p->data.arg_arp_entry));
            msgbuf_p->msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(arg_arp_entry);
            break;

        case NETCFG_MGR_ARP_IPC_GETSTATISTICS:
            msg_p->type.result_ui32 = NETCFG_MGR_ARP_GetStatistics(&(msg_p->data.arg_arp_stat));
            msgbuf_p->msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(arg_arp_stat);
            break;
            
        case NETCFG_MGR_ARP_IPC_GETRUNNINGTIMEOUT:
            msg_p->type.result_running_cfg = NETCFG_MGR_ARP_GetRunningIpNetToMediaTimeout(&(msg_p->data.ui32_v));
            msgbuf_p->msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(ui32_v);
            break;
            
        case NETCFG_MGR_ARP_IPC_GETNEXTRUNNINGSTATICARPENTRY:
            msg_p->type.result_ui32 = NETCFG_MGR_ARP_GetNextStaticIpNetToMediaEntry(&(msg_p->data.arg_arp_static_entry));
            msgbuf_p->msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(arg_arp_static_entry);
            break;
            
        case NETCFG_MGR_ARP_IPC_GETARPENTRY:
            msg_p->type.result_bool = NETCFG_MGR_ARP_GetIpNetToMediaEntry(&(msg_p->data.arg_arp_entry));
            msgbuf_p->msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(arg_arp_entry);
            break;
                
        case NETCFG_MGR_ARP_IPC_SETSTATICARP:
            msg_p->type.result_ui32 = NETCFG_MGR_ARP_SetStaticIpNetToMediaEntry(&(msg_p->data.arg_grp3.arg1), msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_ARP_IPCMSG_TYPE_SIZE;
            break;
                    
        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.result_bool = FALSE;
            msgbuf_p->msg_size = NETCFG_MGR_ARP_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} 

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_AddStaticIpNetToMediaEntry
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
UI32_T NETCFG_MGR_ARP_AddStaticIpNetToMediaEntry(UI32_T vid_ifIndex,
                                                                         UI32_T ip_addr,
                                                                         UI32_T phy_addr_len,
                                                                         UI8_T *phy_addr)
{
    NETCFG_TYPE_StaticIpNetToMediaEntry_T        static_entry;
    NETCFG_TYPE_IPv4RifConfig_T                  rif_config;
    NETCFG_OM_IP_InetRifConfig_T                 rif;
    UI32_T                      tmp_if_index = 0;
    UI32_T                      return_val;
    UI32_T                      type = 0;
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
    
    if ((ip_addr == 0) || (ip_addr == 0xffffffff))
        return NETCFG_TYPE_INVALID_ARG;
    
    if(vid_ifIndex > SYS_ADPT_MAX_VLAN_ID || vid_ifIndex < 0)
        return NETCFG_TYPE_INVALID_ARG;
    
    if((phy_addr == NULL) || (phy_addr_len != NETCFG_TYPE_PHY_ADDRESEE_LENGTH))
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&static_entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
    memset(&rif_config, 0, sizeof(NETCFG_TYPE_IPv4RifConfig_T));
    memset(&rif,0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
    
    memcpy(rif_config.ip_addr, &ip_addr, sizeof(UI8_T)*SYS_ADPT_IPV4_ADDR_LEN);
    
    if (IP_LIB_IsMulticastIp(rif_config.ip_addr) || IP_LIB_IsLoopBackIp(rif_config.ip_addr))
        return NETCFG_TYPE_INVALID_ARG;
    
    if(NETCFG_MGR_ARP_CheckPhysicalAddress(phy_addr) != TRUE)
        return NETCFG_TYPE_INVALID_ARG;
    
    if(arpcfg_static_arp_num >= SYS_ADPT_MAX_NBR_OF_STATIC_ARP_CACHE_ENTRY)
        return NETCFG_TYPE_TABLE_FULL;
    
    rif.addr.u.prefix4.s_addr = ip_addr;

    if(NETCFG_OM_IP_GetInetRif(&rif) == NETCFG_TYPE_OK)
        return NETCFG_TYPE_CAN_NOT_ADD_LOCAL_IP;

    if(NETCFG_OM_IP_GetRifFromSubnet(&rif_config) != NETCFG_TYPE_OK)
    {   
        /* Can't find any routing interface which the IP belong to,
              * reject the request
             */
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
    }
    else
    {
        tmp_if_index = rif_config.ifindex;
    }
        
    

    if(tmp_if_index == 0)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
    
    if(vid_ifIndex == 0)
    {   
        vid_ifIndex = tmp_if_index;
    }
    else if(vid_ifIndex != tmp_if_index)
        return NETCFG_TYPE_INVALID_ARG; 

    if (NETCFG_MGR_ARP_GetARPEntryType(ip_addr, &type))
    {
        if (type == VAL_ipNetToMediaExtType_vrrp)
            return NETCFG_TYPE_IP_IS_VRRP_ADDRESS;
    }

    static_entry.ip_net_to_media_entry.ip_net_to_media_if_index    = vid_ifIndex;
    static_entry.ip_net_to_media_entry.ip_net_to_media_net_address = ip_addr;

    if(NETCFG_OM_ARP_GetStaticEntry(&static_entry) == TRUE)
    {
        return NETCFG_TYPE_ENTRY_EXIST;
    }
    /*check end*/

    return_val = ARP_MGR_AddStaticIpNetToMediaEntry(vid_ifIndex,ip_addr,phy_addr_len,phy_addr);
    if(return_val != NETCFG_TYPE_OK)
        return return_val;
    
    memcpy(static_entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string, phy_addr, phy_addr_len);
    static_entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_type = SN_LAN;
    static_entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_len = phy_addr_len;
    static_entry.ip_net_to_media_entry.ip_net_to_media_type = VAL_ipNetToMediaType_static;
    static_entry.status = TRUE;
    
    if(NETCFG_OM_ARP_AddStaticEntry(&static_entry) == TRUE)
    {
        arpcfg_static_arp_num++;
        return NETCFG_TYPE_OK;
    }
    else
    {
        ARP_MGR_DeleteStaticIpNetToMediaEntry(vid_ifIndex, ip_addr);
        return NETCFG_TYPE_CAN_NOT_ADD;
    }  

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_DeleteStaticIpNetToMediaEntry
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
UI32_T NETCFG_MGR_ARP_DeleteStaticIpNetToMediaEntry(UI32_T vid_ifIndex, UI32_T ip_addr)
{
    NETCFG_TYPE_StaticIpNetToMediaEntry_T     entry;
    NETCFG_TYPE_IPv4RifConfig_T                 rif_config;
    UI32_T                  tmp_if_index=0;
    UI32_T                     return_value;
    BOOL_T                     rc;
    
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
    
    if ((ip_addr == 0) || (ip_addr == 0xffffffff))
        return NETCFG_TYPE_INVALID_ARG;
    
    if(vid_ifIndex > SYS_ADPT_MAX_VLAN_ID || vid_ifIndex < 0)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
    memset(&rif_config, 0, sizeof(NETCFG_TYPE_IPv4RifConfig_T));
    memcpy(rif_config.ip_addr, &ip_addr, sizeof(UI8_T)*SYS_ADPT_IPV4_ADDR_LEN);
    
    if (IP_LIB_IsMulticastIp(rif_config.ip_addr) || IP_LIB_IsLoopBackIp(rif_config.ip_addr))
        return NETCFG_TYPE_INVALID_ARG;
    /*check end*/
    

    if(NETCFG_OM_IP_GetRifFromSubnet(&rif_config) != NETCFG_TYPE_OK)
    {
        while(NETCFG_OM_ARP_GetNextStaticEntry(&entry) == TRUE)
        {
            if(entry.ip_net_to_media_entry.ip_net_to_media_net_address == ip_addr)
            {
                if(NETCFG_OM_ARP_DeleteStaticEntry(&entry) == FALSE)
                {
                    return NETCFG_TYPE_CAN_NOT_DELETE;
                }
                else
                {
                    arpcfg_static_arp_num--;
                    return NETCFG_TYPE_OK;
                }
            }
        }
        return NETCFG_TYPE_OK;
    }
    else
    {
        tmp_if_index = rif_config.ifindex;

        if(tmp_if_index == 0)
            return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
        
        if(vid_ifIndex == 0)
        {
            vid_ifIndex = tmp_if_index;
        }
        else if(vid_ifIndex != tmp_if_index)
        {
            return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
        }

        entry.ip_net_to_media_entry.ip_net_to_media_if_index       = vid_ifIndex;
        entry.ip_net_to_media_entry.ip_net_to_media_net_address    = ip_addr;

        if(NETCFG_OM_ARP_GetStaticEntry(&entry) == FALSE)
        {
            return NETCFG_TYPE_ENTRY_NOT_EXIST;
        } 
        
        return_value = ARP_MGR_DeleteStaticIpNetToMediaEntry(vid_ifIndex, ip_addr);
        if(return_value != NETCFG_TYPE_OK)
            return return_value;
        
        rc = NETCFG_OM_ARP_DeleteStaticEntry(&entry);
        if(rc == FALSE)
        {
            ARP_MGR_AddStaticIpNetToMediaEntry(vid_ifIndex,
                                               ip_addr,
                                               entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_len,
                                               entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string);
            return NETCFG_TYPE_CAN_NOT_DELETE;
        }
        else
        {
            arpcfg_static_arp_num--;
            return NETCFG_TYPE_OK;
        }
    }
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_GetNextStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE :Lookup next static ARP entry.
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
UI32_T NETCFG_MGR_ARP_GetNextStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
    /*check end*/
    
    if ( TRUE == NETCFG_OM_ARP_GetNextStaticEntry(entry))
        return NETCFG_TYPE_OK;
    else
        return NETCFG_TYPE_FAIL;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_SetIpNetToMediaTimeout
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
UI32_T NETCFG_MGR_ARP_SetIpNetToMediaTimeout(UI32_T age_time)
{
    UI32_T      ret;
    UI32_T      pre_timeout;
    
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(age_time < SYS_ADPT_MIN_ARP_CACHE_TIMEOUT || age_time > SYS_ADPT_MAX_ARP_CACHE_TIMEOUT)
        return NETCFG_TYPE_INVALID_ARG;
    /*check end*/
    
    NETCFG_OM_ARP_GetTimeout(&pre_timeout);

    ret = ARP_MGR_SetIpNetToMediaEntryTimeout(age_time,pre_timeout);
    
    if (ret != NETCFG_TYPE_OK)
    {
        return ret;
    }

    if(NETCFG_OM_ARP_SetTimeout(age_time) == FALSE)
    {
        ARP_MGR_SetIpNetToMediaEntryTimeout(pre_timeout,pre_timeout);
        return NETCFG_TYPE_CAN_NOT_SET;
    }

    return NETCFG_TYPE_OK;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_DeleteAllDynamicIpNetToMediaEntry
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
UI32_T NETCFG_MGR_ARP_DeleteAllDynamicIpNetToMediaEntry(void)
{
    UI32_T      ret;
    
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
    /*check end*/

    ret = ARP_MGR_DeleteAllDynamicIpNetToMediaEntry();
    
    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_GetIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Get an ARP entry information.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :key are ifindex and ip address.
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_GetNextIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry)
{
    UI32_T      ret;
    
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
    /*check end*/

    ret = ARP_MGR_GetNextIpNetToMediaEntry(entry);
    
    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_GetStatistics
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
UI32_T NETCFG_MGR_ARP_GetStatistics(NETCFG_TYPE_IpNetToMedia_Statistics_T *stat)
{
    UI32_T        ret;
    
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
    /*check end*/

    ret = ARP_MGR_GetStatistic(stat);
    
    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_GetRunningIpNetToMediaTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ARP timeout.
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
NETCFG_MGR_ARP_GetRunningIpNetToMediaTimeout(UI32_T *age_time)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    /*check end*/
    
    if ( TRUE == NETCFG_OM_ARP_GetTimeout(age_time))
    {
        if (SYS_DFLT_IP_NET_TO_MEDIA_ENTRY_TIMEOUT != *age_time)
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_GetIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Get an ARP entry information.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :TRUE/
 *                 FALSE
 *
 * NOTES   :key are ifindex and ip address.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_MGR_ARP_GetIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry)
{
    UI32_T        ret;
    
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;
    /*check end*/

    ret = ARP_MGR_GetIpNetToMediaEntry(entry);
    
    if (ret != NETCFG_TYPE_OK)
        return FALSE;
    else    
        return TRUE;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_SetStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Set a static ARP entry invalid or valid.
 *
 * INPUT   : entry, type.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_SetStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry, int type)
{
    UI32_T        ret;
    
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    switch(type)
    {
        case  VAL_ipNetToMediaType_invalid :
              ret = NETCFG_MGR_ARP_DeleteStaticIpNetToMediaEntry(entry->ip_net_to_media_entry.ip_net_to_media_if_index,
                                                                 entry->ip_net_to_media_entry.ip_net_to_media_net_address);
        break;
        case  VAL_ipNetToMediaType_static :
            ret = NETCFG_MGR_ARP_AddStaticIpNetToMediaEntry(entry->ip_net_to_media_entry.ip_net_to_media_if_index,
                                                            entry->ip_net_to_media_entry.ip_net_to_media_net_address,
                                                            entry->ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_len,
                                                            entry->ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string);
        break;
        case  VAL_ipNetToMediaType_dynamic :
        case  VAL_ipNetToMediaType_other :
        default :
            ret = NETCFG_TYPE_NOT_IMPLEMENT;
        break;
    }
    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_SignalRifCreate
 *-----------------------------------------------------------------------------
 * PURPOSE : when create a rif signal ARP .
 *
 * INPUT   : ip_addr, ip_mask.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :When rif create only update ifindex.
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_SignalRifCreate(UI32_T ip_addr, UI32_T ip_mask)
{
    NETCFG_TYPE_StaticIpNetToMediaEntry_T     entry;
    NETCFG_TYPE_StaticIpNetToMediaEntry_T     temp_entry;
    NETCFG_OM_IP_InetRifConfig_T              rif;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(ip_addr == 0 || ip_mask == 0)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
    memset(&temp_entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
    memset(&rif, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
    rif.addr.u.prefix4.s_addr = ip_addr;

    if(NETCFG_OM_IP_GetInetRif(&rif) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_INVALID_IP;
    
    while(NETCFG_OM_ARP_GetNextStaticEntryByAddress(&entry) == TRUE)
    {
        if((entry.ip_net_to_media_entry.ip_net_to_media_net_address & ip_mask) == (ip_addr & ip_mask))
        {
            memcpy(&temp_entry, &entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
            NETCFG_OM_ARP_DeleteStaticEntry(&entry);
            temp_entry.ip_net_to_media_entry.ip_net_to_media_if_index = rif.ifindex;
            memcpy(&entry, &temp_entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
            NETCFG_OM_ARP_AddStaticEntry(&entry);
        }
    }
    return NETCFG_TYPE_OK;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_SignalRifActive
 *-----------------------------------------------------------------------------
 * PURPOSE : when an exist rif active signal ARP .
 *
 * INPUT   : ip_addr, ip_mask.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :Update entry status and add or remove the entry from amtrl3/ipal.
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_SignalRifActive(UI32_T ip_addr, UI32_T ip_mask)
{
    NETCFG_TYPE_StaticIpNetToMediaEntry_T     entry;
    NETCFG_TYPE_StaticIpNetToMediaEntry_T     temp_entry;
    UI32_T                      rc;
    
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(ip_addr == 0 || ip_mask == 0)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
    memset(&temp_entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
   
    while(NETCFG_OM_ARP_GetNextStaticEntry(&entry) == TRUE)
    {
        if((entry.ip_net_to_media_entry.ip_net_to_media_net_address & ip_mask) == (ip_addr & ip_mask))
        {
            if((entry.ip_net_to_media_entry.ip_net_to_media_net_address == ip_addr) && (entry.status == TRUE))
            {
                rc = ARP_MGR_DeleteStaticIpNetToMediaEntry(entry.ip_net_to_media_entry.ip_net_to_media_if_index,
                                                           entry.ip_net_to_media_entry.ip_net_to_media_net_address);
                if(rc != NETCFG_TYPE_OK)
                    continue;
                else
                {
                    memcpy(&temp_entry, &entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
                    NETCFG_OM_ARP_DeleteStaticEntry(&entry);
                    temp_entry.status = FALSE;
                    memcpy(&entry, &temp_entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
                    NETCFG_OM_ARP_AddStaticEntry(&entry);
                }
            }
            else if((entry.ip_net_to_media_entry.ip_net_to_media_net_address != ip_addr) && (entry.status == FALSE))
            {
                rc = ARP_MGR_AddStaticIpNetToMediaEntry(entry.ip_net_to_media_entry.ip_net_to_media_if_index,
                                                        entry.ip_net_to_media_entry.ip_net_to_media_net_address,
                                                        entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_len,
                                                        entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string);
                if(rc != NETCFG_TYPE_OK)
                    continue;
                else
                {
                    memcpy(&temp_entry, &entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
                    NETCFG_OM_ARP_DeleteStaticEntry(&entry);
                    temp_entry.status = TRUE;
                    memcpy(&entry, &temp_entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
                    NETCFG_OM_ARP_AddStaticEntry(&entry);
                }
            }
        }
    }
    return NETCFG_TYPE_OK;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_SignalRifDown
 *-----------------------------------------------------------------------------
 * PURPOSE : when a rif down signal ARP .
 *
 * INPUT   : ip_addr, ip_mask.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :Update entry status and add or remove the entry from amtrl3/ipal.
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_SignalRifDown(UI32_T ip_addr, UI32_T ip_mask)
{
    NETCFG_TYPE_StaticIpNetToMediaEntry_T      entry;
    NETCFG_TYPE_StaticIpNetToMediaEntry_T      temp_entry;
    NETCFG_OM_IP_InetRifConfig_T               rif;
    NETCFG_OM_IP_Interface_T                   vlan_intf;
    UI32_T                    rc;
    UI32_T                    mask;
    UI32_T                    ifindex;
    BOOL_T                    if_network_has_other_addr = FALSE;
    
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(ip_addr == 0 || ip_mask == 0)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
    memset(&temp_entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
    memset(&rif, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
    memset(&vlan_intf, 0, sizeof (NETCFG_OM_IP_Interface_T));
    rif.addr.u.prefix4.s_addr = ip_addr;

    if(NETCFG_OM_IP_GetInetRif(&rif) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_INVALID_IP;

    ifindex = rif.ifindex;
    memset(&rif, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
    
    if (NETCFG_OM_IP_GetInterfaceByIfindex(ifindex, &vlan_intf) == FALSE)
        return NETCFG_TYPE_INVALID_IP;
    
    if((vlan_intf.if_flags) & IFF_RUNNING)/*check if the interface active*/
    {
        while(NETCFG_OM_IP_GetNextInetRifByIfindex(&rif,ifindex) == NETCFG_TYPE_OK)
        {
            mask = PREFIXLEN2MASK(rif.addr.prefixlen);
            if((rif.addr.u.prefix4.s_addr & mask) == (ip_addr & ip_mask))
            {
                if(rif.addr.u.prefix4.s_addr != ip_addr)
                {
                    if_network_has_other_addr = TRUE;/*the network have other active rif*/
                    break;
                }
            }
        }
    }
    
    while(NETCFG_OM_ARP_GetNextStaticEntry(&entry) == TRUE)
    {
        if((entry.ip_net_to_media_entry.ip_net_to_media_net_address & ip_mask) == (ip_addr & ip_mask))
        {
            if((entry.ip_net_to_media_entry.ip_net_to_media_net_address == ip_addr) && (if_network_has_other_addr == TRUE))
            {
                rc = ARP_MGR_AddStaticIpNetToMediaEntry(entry.ip_net_to_media_entry.ip_net_to_media_if_index,
                                                        entry.ip_net_to_media_entry.ip_net_to_media_net_address,
                                                        entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_len,
                                                        entry.ip_net_to_media_entry.ip_net_to_media_phys_address.phy_address_cctet_string);
                if(rc != NETCFG_TYPE_OK)
                    continue;
                else
                {
                    memcpy(&temp_entry, &entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
                    NETCFG_OM_ARP_DeleteStaticEntry(&entry);
                    temp_entry.status = TRUE;
                    memcpy(&entry, &temp_entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
                    NETCFG_OM_ARP_AddStaticEntry(&entry);
                }
            }
            else if((entry.ip_net_to_media_entry.ip_net_to_media_net_address != ip_addr) && (if_network_has_other_addr == FALSE))
            {
                rc = ARP_MGR_DeleteStaticIpNetToMediaEntry(entry.ip_net_to_media_entry.ip_net_to_media_if_index,
                                                           entry.ip_net_to_media_entry.ip_net_to_media_net_address);
                if(rc != NETCFG_TYPE_OK)
                    continue;
                else
                {
                    memcpy(&temp_entry, &entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
                    NETCFG_OM_ARP_DeleteStaticEntry(&entry);
                    temp_entry.status = FALSE;
                    memcpy(&entry, &temp_entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
                    NETCFG_OM_ARP_AddStaticEntry(&entry);
                }        
            }
        }
    }
    return NETCFG_TYPE_OK;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_SignalRifDestroy
 *-----------------------------------------------------------------------------
 * PURPOSE : when destroy a rif signal ARP .
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
UI32_T NETCFG_MGR_ARP_SignalRifDestroy(UI32_T ip_addr, UI32_T ip_mask)
{
    NETCFG_TYPE_StaticIpNetToMediaEntry_T      entry;
    NETCFG_TYPE_StaticIpNetToMediaEntry_T      temp_entry;
    NETCFG_TYPE_IPv4RifConfig_T                rif_config;
    BOOL_T                    if_network_has_other_addr = FALSE;
    
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(ip_addr == 0 || ip_mask == 0)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
    memset(&temp_entry, 0, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
    memcpy(rif_config.ip_addr, &ip_addr, sizeof(UI8_T)*SYS_ADPT_IPV4_ADDR_LEN);

    if(NETCFG_OM_IP_GetRifFromSubnet(&rif_config) == NETCFG_TYPE_OK)
        if_network_has_other_addr = TRUE;/*the network have other ip address*/
    
    while(NETCFG_OM_ARP_GetNextStaticEntryByAddress(&entry) == TRUE)
    {
        if((entry.ip_net_to_media_entry.ip_net_to_media_net_address & ip_mask) == (ip_addr & ip_mask))
        {
            if(if_network_has_other_addr == FALSE)
            {
                memcpy(&temp_entry, &entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
                NETCFG_OM_ARP_DeleteStaticEntry(&entry);
                temp_entry.ip_net_to_media_entry.ip_net_to_media_if_index = 0;
                memcpy(&entry, &temp_entry, sizeof(NETCFG_TYPE_StaticIpNetToMediaEntry_T));
                NETCFG_OM_ARP_AddStaticEntry(&entry);
            }
        }
    }
    return NETCFG_TYPE_OK;
}

