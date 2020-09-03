/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_MGR_ND.C
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
#include "l_threadgrp.h"
#include "ip_lib.h"
#include "netcfg_type.h"
#include "netcfg_mgr_nd.h"
#include "netcfg_pmgr_nd.h" // for backdroor
#include "leaf_1213.h"
#include "leaf_es3626a.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "nd_mgr.h"
#include "netcfg_om_nd.h"
#include "netcfg_om_ip.h"
#include "route_mgr.h"
#include "swctrl_pmgr.h"
#ifdef  BACKDOOR_OPEN
#include "backdoor_mgr.h"
#endif
#include "l_threadgrp.h"
#include "string.h"
#include "stdlib.h"
#include "netcfg_proc_comm.h"
#if (SYS_CPNT_NSM == TRUE)
#include "nsm_pmgr.h"
#endif
#include "ipal_neigh.h"
#include "ipal_icmp.h"

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    #include "swctrl_pom.h"
    #include "trk_pmgr.h"
    #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
        #include "sys_module.h"
        #include "amtr_pmgr.h"
        #include "vlan_pom.h"
        #include "l2mux_pmgr.h"
    #endif
#endif

SYSFUN_DECLARE_CSC

static BOOL_T is_provision_complete = FALSE;

#define SN_LAN                          1
#define PREFIXLEN2MASK(len)             ~((1 << (32 - len)) - 1)
#define IS_MULTICAST_MAC(mac)           ((mac)[0] & 0x01)
#define IS_ZERO_MAC(mac)                ((mac[0]|mac[1]|mac[2]|mac[3]|mac[4]|mac[5]) == 0)
#define IS_VALID_MAC(mac)               (!IS_MULTICAST_MAC(mac) && !IS_ZERO_MAC(mac)) /* broadcast MAC is also a multicast MAC */

static int NETCFG_MGR_ND_Backdoor_GetIP(UI32_T * IPaddr);
static int NETCFG_MGR_ND_Backdoor_AtoIP(UI8_T *s, UI8_T *ip);
static void NETCFG_MGR_ND_Backdoor_SignalRifCreate(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ND_Backdoor_SignalRifDestroy(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ND_Backdoor_SignalRifUp(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ND_Backdoor_SignalRifDown(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ND_Backdoor_GetStaticEntry(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ND_Backdoor_GetAllStaticEntry(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ND_Backdoor_DeleteAllStaticEntry(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ND_Backdoor_Ipv6UnitTest(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
static void NETCFG_MGR_ND_Backdoor_Main(void);
static BOOL_T NETCFG_MGR_ND_GetNDEntryType(L_INET_AddrIp_T* ip_addr, UI32_T *entry_type);
static int NETCFG_MGR_prefix_compare(UI8_T* input1,UI8_T* input2, int prefix_len) ;
static L_INET_AddrIp_T NETCFG_MGR_ComposeInetAddr(UI16_T type,UI8_T* addrp,UI16_T prefixLen);
///////////////////////////////////
//static UI32_T NETCFG_MGR_ND_AddStaticIpv6NetToMediaEntry(UI32_T vid_ifindex,
//                                                                         L_INET_AddrIp_T *ip_addr,
//                                                                         NETCFG_TYPE_PhysAddress_T *phy_addr);
//
//static UI32_T NETCFG_MGR_ND_DeleteStaticIpv6NetToMediaEntry(UI32_T vid_ifindex, L_INET_AddrIp_T* ip_addr);
//static BOOL_T NETCFG_MGR_ND_GetIpv6NetToMediaEntry(NETCFG_TYPE_Ipv6NetToMediaEntry_T *entry);
//static UI32_T NETCFG_MGR_ND_GetNextStaticIpv6NetToMediaEntry(NETCFG_TYPE_StaticIpv6NetToMediaEntry_T *entry);
//static UI32_T NETCFG_MGR_ND_GetNextIpv6NetToMediaEntry(NETCFG_TYPE_Ipv6NetToMediaEntry_T *entry);
//static UI32_T NETCFG_MGR_ND_DeleteAllDynamicIpv6NetToMediaEntry(void);
//static UI32_T NETCFG_MGR_ND_SetStaticIpv6NetToMediaEntry(NETCFG_TYPE_StaticIpv6NetToMediaEntry_T *entry, int type);


#define NETCFG_MGR_PREFIX_COMPARE(addr1,addr2,prefixLen) NETCFG_MGR_prefix_compare(addr1,addr2,prefixLen) /*temp comp fun, maybe replaced by library func??*/
#define NETCFG_MGR_COMPOSEINETADDR(type,addrp,prefixLen) NETCFG_MGR_ComposeInetAddr(type, addrp,prefixLen) /*temp comp fun, maybe replaced by library func??*/


/*Simon's debug function*/
#define DEBUG_FLAG_BIT_DEBUG 0x01
#define DEBUG_FLAG_BIT_INFO  0x02
#define DEBUG_FLAG_BIT_NOTE  0x04
/***************************************************************/
static UI32_T DEBUG_FLAG = 0;
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

static char  ip_string_buffer[L_INET_MAX_IPADDR_STR_LEN+1];
/*END Simon's debug function*/

static int NETCFG_MGR_ND_Backdoor_AtoIP(UI8_T *s, UI8_T *ip)
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

static int NETCFG_MGR_ND_Backdoor_GetIP(UI32_T * IPaddr)
{
    int ret = 0;
    UI8_T   *temp_ip = (UI8_T *)IPaddr;
    UI8_T   buffer[20] = {0};

    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    ret = NETCFG_MGR_ND_Backdoor_AtoIP(buffer, temp_ip);

    return  ret;
}

/* FUNCTION NAME : NETCFG_MGR_ND_Backdoor_SignalRifCreate
 * PURPOSE:Backdoor of when create a rif signal ND
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
static void NETCFG_MGR_ND_Backdoor_SignalRifCreate(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI32_T ip_addr;
    //UI32_T ip_mask;
    UI32_T ret;
    UI8_T line_buffer[64];
    L_INET_AddrIp_T addr;
    int value;

    BACKDOOR_MGR_Printf("\n\r Input Create IP address(x.x.x.x): ");
    if(!NETCFG_MGR_ND_Backdoor_GetIP(&ip_addr))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP address\n");

    //BACKDOOR_MGR_Printf("\n\r Input IP Mask(x.x.x.x): ");
    //if(!NETCFG_MGR_ND_Backdoor_GetIP(&ip_mask))
    //   BACKDOOR_MGR_Printf("\nYou entered an invalid IP mask\n");

    BACKDOOR_MGR_Printf("\n\r Input IP prefix length (bit): ");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 3);
    value = atoi((char*)line_buffer);
    if(value >128 || value <0)
    {
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP prefix\n");
        return ;
    }
    addr = NETCFG_MGR_COMPOSEINETADDR(L_INET_ADDR_TYPE_IPV4,(UI8_T*)&ip_addr,value);


    L_THREADGRP_Execution_Request(tg_handle, member_id);
    ret = NETCFG_MGR_ND_SignalRifCreate(&addr);
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

/* FUNCTION NAME : NETCFG_MGR_ND_Backdoor_SignalRifDestroy
 * PURPOSE:Backdoor of when destroy a rif signal ND
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
static void NETCFG_MGR_ND_Backdoor_SignalRifDestroy(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI32_T ip_addr;
    //UI32_T ip_mask;
    UI32_T ret;
    UI8_T line_buffer[64];
    L_INET_AddrIp_T addr;
    int value;

    BACKDOOR_MGR_Printf("\n\r Input Destroy IP address(x.x.x.x): ");
    if(!NETCFG_MGR_ND_Backdoor_GetIP(&ip_addr))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP address\n");

    //BACKDOOR_MGR_Printf("\n\r Input IP Mask(x.x.x.x): ");
    //if(!NETCFG_MGR_ND_Backdoor_GetIP(&ip_mask))
    //    BACKDOOR_MGR_Printf("\nYou entered an invalid IP mask\n");

    BACKDOOR_MGR_Printf("\n\r Input IP prefix length (bit): ");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 3);
    value = atoi((char*)line_buffer);
    if(value >128 || value <0)
    {
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP prefix\n");
        return ;
    }
    addr = NETCFG_MGR_COMPOSEINETADDR(L_INET_ADDR_TYPE_IPV4,(UI8_T*)&ip_addr,value);

    L_THREADGRP_Execution_Request(tg_handle, member_id);
    ret = NETCFG_MGR_ND_SignalRifDestroy(&addr);
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


/* FUNCTION NAME : NETCFG_MGR_ND_Backdoor_SignalRifUp
 * PURPOSE:Backdoor of when  a rif up signal ND
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
static void NETCFG_MGR_ND_Backdoor_SignalRifUp(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI32_T ifindex;
    UI32_T ip_addr;
    UI32_T ret;
    UI8_T line_buffer[64];
    L_INET_AddrIp_T addr;
    int value;

    BACKDOOR_MGR_Printf("\n\r Input ifindex: ");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 4);
    ifindex = atoi((char*)line_buffer);

    BACKDOOR_MGR_Printf("\n\r Input Up IP address(x.x.x.x): ");
    if(!NETCFG_MGR_ND_Backdoor_GetIP(&ip_addr))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP address\n");

    BACKDOOR_MGR_Printf("\n\r Input IP prefix length (bit): ");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 3);
    value = atoi((char*)line_buffer);
    if(value >128 || value <0)
    {
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP prefix\n");
        return ;
    }
    addr = NETCFG_MGR_COMPOSEINETADDR(L_INET_ADDR_TYPE_IPV4,(UI8_T*)&ip_addr,value);

    L_THREADGRP_Execution_Request(tg_handle, member_id);
    ret = NETCFG_MGR_ND_SignalRifUp(ifindex, &addr);
    L_THREADGRP_Execution_Release(tg_handle, member_id);
    switch(ret)
    {
        case NETCFG_TYPE_NOT_MASTER_MODE:
            BACKDOOR_MGR_Printf("\nNot in master mode\n");
            break;

        case NETCFG_TYPE_INVALID_ARG:
            BACKDOOR_MGR_Printf("\nInvalid IP address or IP mask\n");
            break;

        case NETCFG_TYPE_INTERFACE_NOT_EXISTED:
            BACKDOOR_MGR_Printf("\nThe Rif not up\n");
            break;


        case NETCFG_TYPE_OK:
            BACKDOOR_MGR_Printf("\nOK!!!\n");
            break;

        default:
            BACKDOOR_MGR_Printf("\nFail\n");
            break;
    }
}


/* FUNCTION NAME : NETCFG_MGR_ND_Backdoor_SignalRifDown
 * PURPOSE:Backdoor of when  a rif down signal ND
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
static void NETCFG_MGR_ND_Backdoor_SignalRifDown(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI32_T ifindex;
    UI32_T ip_addr;
    int value;
    UI32_T ret;
    UI8_T line_buffer[64];
    L_INET_AddrIp_T addr;

    BACKDOOR_MGR_Printf("\n\r Input ifindex: ");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 4);
    ifindex = atoi((char*)line_buffer);

    BACKDOOR_MGR_Printf("\n\r Input Down IP address(x.x.x.x): ");
    if(!NETCFG_MGR_ND_Backdoor_GetIP(&ip_addr))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP address\n");

    BACKDOOR_MGR_Printf("\n\r Input IP prefix length (bit): ");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 3);
    value = atoi((char*)line_buffer);
    if(value >128 || value <0)
    {
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP prefix\n");
        return ;
    }
    addr = NETCFG_MGR_COMPOSEINETADDR(L_INET_ADDR_TYPE_IPV4,(UI8_T* )&ip_addr,value);
    L_THREADGRP_Execution_Request(tg_handle, member_id);
    ret = NETCFG_MGR_ND_SignalRifDown(ifindex,&addr);
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

/* FUNCTION NAME : NETCFG_MGR_ND_Backdoor_GetStaticEntry
 * PURPOSE:
 *      Backdoor for get a static nd entry.
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
static void NETCFG_MGR_ND_Backdoor_GetStaticEntry(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI32_T  ip_addr;
    UI32_T  ifindex;
    char    *terminal;
    char    buf[16];
    BOOL_T  ret;
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T entry;

    memset(&entry, 0, sizeof(entry));

    BACKDOOR_MGR_Printf("\n\r Input IP address(x.x.x.x): ");
    if(!NETCFG_MGR_ND_Backdoor_GetIP(&ip_addr))
        BACKDOOR_MGR_Printf("\nYou entered an invalid IP address\n");

    BACKDOOR_MGR_Printf("\n\r Input Ifindex: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 15);
    ifindex = (UI32_T) strtoul(buf, &terminal, 10);
    BACKDOOR_MGR_Printf ("\n");

    entry.ip_net_to_physical_entry.ip_net_to_physical_if_index = ifindex;
    memcpy(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr,&ip_addr,4);
    entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.type = L_INET_ADDR_TYPE_IPV4;

    L_THREADGRP_Execution_Request(tg_handle, member_id);
    ret = NETCFG_OM_ND_GetStaticEntry(&entry);
    L_THREADGRP_Execution_Release(tg_handle, member_id);

    if(ret == TRUE)
    {
        BACKDOOR_MGR_Printf("\n\r The static ND information: ");
        BACKDOOR_MGR_Printf("\n\r Ifindex: %d", entry.ip_net_to_physical_entry.ip_net_to_physical_if_index);
        BACKDOOR_MGR_Printf("\n\r IP address: %d.%d.%d.%d", L_INET_EXPAND_IPV4(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr));
        BACKDOOR_MGR_Printf("\n\r Physical address: %02X-%02X-%02X-%02X-%02X-%02X", L_INET_EXPAND_MAC(entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string));
        if(entry.status == TRUE)
            BACKDOOR_MGR_Printf("\n\r Status: TRUE");
        else
            BACKDOOR_MGR_Printf("\n\r Status: FALSE");
    }
    else
        BACKDOOR_MGR_Printf("\n\r Cannot find the static ND entry.");
}


/* FUNCTION NAME : NETCFG_MGR_ND_Backdoor_GetAllStaticEntry
 * PURPOSE:
 *      Backdoor for get all of static nd entries.
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
static void NETCFG_MGR_ND_Backdoor_GetAllStaticEntry(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T entry;


    L_THREADGRP_Execution_Request(tg_handle, member_id);
    while(NETCFG_OM_ND_GetNextStaticEntry(L_INET_ADDR_TYPE_IPV4,&entry) == TRUE)
    {
        BACKDOOR_MGR_Printf("\n\r");
        BACKDOOR_MGR_Printf("\n\r Ifindex: %d", entry.ip_net_to_physical_entry.ip_net_to_physical_if_index);
        BACKDOOR_MGR_Printf("\n\r IP address: %d.%d.%d.%d", L_INET_EXPAND_IPV4(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr));
        BACKDOOR_MGR_Printf("\n\r Physical address: %02X-%02X-%02X-%02X-%02X-%02X", L_INET_EXPAND_MAC(entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string));
        if(entry.status == TRUE)
            BACKDOOR_MGR_Printf("\n\r Status: TRUE");
        else
            BACKDOOR_MGR_Printf("\n\r Status: FALSE");
    }
    while(NETCFG_OM_ND_GetNextStaticEntry(L_INET_ADDR_TYPE_IPV6,&entry) == TRUE)
    {
        BACKDOOR_MGR_Printf("\n\r");
        BACKDOOR_MGR_Printf("\n\r Ifindex: %d", entry.ip_net_to_physical_entry.ip_net_to_physical_if_index);
        BACKDOOR_MGR_Printf("\n\r IP address: %d.%d.%d.%d", L_INET_EXPAND_IPV4(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr));
        BACKDOOR_MGR_Printf("\n\r Physical address: %02X-%02X-%02X-%02X-%02X-%02X", L_INET_EXPAND_MAC(entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string));
        if(entry.status == TRUE)
            BACKDOOR_MGR_Printf("\n\r Status: TRUE");
        else
            BACKDOOR_MGR_Printf("\n\r Status: FALSE");
    }
    L_THREADGRP_Execution_Release(tg_handle, member_id);
}

/* FUNCTION NAME : NETCFG_MGR_ND_Backdoor_GetAllStaticEntry
 * PURPOSE:
 *      Backdoor for delete all of static nd entries.
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
static void NETCFG_MGR_ND_Backdoor_DeleteAllStaticEntry(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T entry;

    memset(&entry, 0, sizeof(entry));

    L_THREADGRP_Execution_Request(tg_handle, member_id);
    NETCFG_OM_ND_DeleteAllStaticEntry();

    if(NETCFG_OM_ND_GetNextStaticEntry(L_INET_ADDR_TYPE_IPV4,&entry) == TRUE)
        BACKDOOR_MGR_Printf("\n\r Cannot delete all static ND entries");
    else
        BACKDOOR_MGR_Printf("\n\r There isn't any static ND entry");
    L_THREADGRP_Execution_Release(tg_handle, member_id);
}

static void NETCFG_MGR_ND_Backdoor_Ipv6UnitTest(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    int good_count, bad_count;
    L_THREADGRP_Execution_Request(tg_handle, member_id);
    good_count = bad_count =0;
    BACKDOOR_MGR_Printf("Not implement yet\r\n");
    //step. pmgr->mgr


    L_THREADGRP_Execution_Release(tg_handle, member_id);
}

/* FUNCTION NAME : NETCFG_MGR_ND_BackDoorMain
 * PURPOSE:Backdoor of ND
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
static void NETCFG_MGR_ND_Backdoor_Main(void)
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
        BACKDOOR_MGR_Printf("1.  Signal Rif Create\n");
        BACKDOOR_MGR_Printf("2.  Signal Rif Destroy\n");
        BACKDOOR_MGR_Printf("3.  Signal Rif Up\n");
        BACKDOOR_MGR_Printf("4.  Signal Rif Down\n");
        BACKDOOR_MGR_Printf("5.  Get Static ND Entry\n");
        BACKDOOR_MGR_Printf("6.  Get All Static ND Entry\n");
        BACKDOOR_MGR_Printf("7.  Delete All Static ND Entry\n");
        BACKDOOR_MGR_Printf("8.  IPv6 unit test\n");
        BACKDOOR_MGR_Printf("9.  IPv6 debug flag\n");
        BACKDOOR_MGR_Printf("10.  Dump OM dtatbase\n");

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
        BACKDOOR_MGR_Printf("11.  Dump RA Guard OM\n");
#endif
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
                NETCFG_MGR_ND_Backdoor_SignalRifCreate(tg_handle, backdoor_member_id);
                break;

            case 2:
                NETCFG_MGR_ND_Backdoor_SignalRifDestroy(tg_handle, backdoor_member_id);
                break;

            case 3:
                NETCFG_MGR_ND_Backdoor_SignalRifUp(tg_handle, backdoor_member_id);
                break;

            case 4:
                NETCFG_MGR_ND_Backdoor_SignalRifDown(tg_handle, backdoor_member_id);
                break;

            case 5:
                NETCFG_MGR_ND_Backdoor_GetStaticEntry(tg_handle, backdoor_member_id);
                break;

            case 6:
                NETCFG_MGR_ND_Backdoor_GetAllStaticEntry(tg_handle, backdoor_member_id);
                break;

            case 7:
                NETCFG_MGR_ND_Backdoor_DeleteAllStaticEntry(tg_handle, backdoor_member_id);
                break;
            case 8:
                NETCFG_MGR_ND_Backdoor_Ipv6UnitTest(tg_handle, backdoor_member_id);
                break;
            case 9://open debug flag
                if(DEBUG_FLAG ==0)
                    DEBUG_FLAG=DEBUG_FLAG_BIT_DEBUG;
                else if (DEBUG_FLAG&DEBUG_FLAG_BIT_NOTE)
                    DEBUG_FLAG=0;
                else
                    DEBUG_FLAG = DEBUG_FLAG<<1|1;
                ND_MGR_BackdoorSetDebugFlag(DEBUG_FLAG);
#if (SYS_CPNT_NSM == TRUE)
                NSM_PMGR_BackdoorSetDebugFlag(DEBUG_FLAG);
#endif
                NETCFG_OM_ND_BackdoorSetDebugFlag(DEBUG_FLAG);
                printf("debug flag is now %x\r\n",(unsigned int)DEBUG_FLAG);

                break;
            case 10:
                {
                    BACKDOOR_MGR_Printf("\n database of vid_ifindex?");
                    BACKDOOR_MGR_RequestKeyIn(buf, 15);
                    ch = (int) strtoul(buf, &terminal, 10);
                    NETCFG_OM_ND_BackdoorDumpDB(ch);
                }
                break;

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
            case 11:
                NETCFG_OM_ND_RAGUARD_BackdoorDumpDB();
                break;
#endif
            default:
                ch = 0;
                break;
        }
    }

    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
}

/* FUNCTION NAME : NETCFG_MGR_ND_GetNDEntryType
 * PURPOSE:Get the ND type of an ND entry.
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
static BOOL_T NETCFG_MGR_ND_GetNDEntryType(L_INET_AddrIp_T* ip_addr, UI32_T *entry_type)
{
    NETCFG_OM_ND_RouterRedundancyEntry_T rp_entry;

    memset(&rp_entry, 0, sizeof(rp_entry));
    rp_entry.ip_addr = *ip_addr;

    if(NETCFG_OM_ND_GetRouterRedundancyProtocolIpNetToPhysicalEntry(&rp_entry) == TRUE)
    {
        *entry_type = VAL_ipNetToMediaExtType_vrrp;
        return TRUE;
    }
    else
        return FALSE;
}

#if 0 /* not used now */
/* FUNCTION NAME : NETCFG_MGR_ND_CheckPhysicalAddress
 * PURPOSE:check if ND physical address valid.
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
 * NOTES:ND physical address should not be all-zero, broastcast,multicast,eaps address.
 *
 */
 static BOOL_T   NETCFG_MGR_ND_CheckPhysicalAddress(UI8_T *phy_addr)
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
#endif /* #if 0 */

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
BOOL_T NETCFG_MGR_ND_InitiateProcessResources(void)
{
    NETCFG_OM_ND_Init();
    ND_MGR_InitiateProcessResources();
    return TRUE;
}

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
void NETCFG_MGR_ND_Create_InterCSC_Relation(void)
{
#ifdef  BACKDOOR_OPEN
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("ndcfg",
                                                      SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
                                                      NETCFG_MGR_ND_Backdoor_Main);
#endif
}

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
void NETCFG_MGR_ND_EnterMasterMode (void)
{
    NETCFG_OM_ND_Init();
    ND_MGR_SetIpNetToMediaEntryTimeout(SYS_DFLT_IP_NET_TO_MEDIA_ENTRY_TIMEOUT,0);
#if (SYS_CPNT_NDSNP == TRUE)
    SWCTRL_PMGR_EnableNdPacketTrap(SWCTRL_ND_TRAP_BY_NETCFG_ND);
#endif
    SYSFUN_ENTER_MASTER_MODE();
}

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
void NETCFG_MGR_ND_ProvisionComplete(void)
{
    is_provision_complete = TRUE;
}

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
void NETCFG_MGR_ND_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

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
void NETCFG_MGR_ND_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

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
 /*Donny.li modify for ND stacking.2008.08.07 */
void NETCFG_MGR_ND_EnterTransitionMode (void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    is_provision_complete = FALSE;
    NETCFG_OM_ND_SetTimeout(SYS_DFLT_IP_NET_TO_MEDIA_ENTRY_TIMEOUT);
    NETCFG_OM_ND_DeleteAllStaticEntry();

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    NETCFG_OM_ND_RAGUARD_ClearOm();
#endif
}
/*Donny.li end modify for ND stacking.2008.08.07 */

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
BOOL_T NETCFG_MGR_ND_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    NETCFG_MGR_ND_IPCMsg_T *msg_p;
    //BOOL_T arg_bool;

    if (msgbuf_p == NULL)
    {
        DBGprintf("null message buffer!");
        return FALSE;
    }

    msg_p = (NETCFG_MGR_ND_IPCMsg_T*)msgbuf_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        msg_p->type.result_bool = FALSE;
        msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
        return TRUE;
    }
    switch (msg_p->type.cmd)
    {
        case NETCFG_MGR_ND_IPC_CREATESTATIC:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_AddStaticIpNetToPhysicalEntry(msg_p->data.arg_nd_config.vid_ifindex,
                                                        &msg_p->data.arg_nd_config.ip_addr,
                                                        msg_p->data.arg_nd_config.phy_addr.phy_address_len,
                                                        msg_p->data.arg_nd_config.phy_addr.phy_address_cctet_string);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_ND_IPC_DELETESTATIC:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_DeleteStaticIpNetToPhysicalEntry(msg_p->data.arg_nd_config.vid_ifindex, &msg_p->data.arg_nd_config.ip_addr);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_ND_IPC_DELETEALLDYNAMIC:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_DeleteAllDynamicIpNetToPhysicalEntry(ND_MGR_GET_FLAGS_IPV4|ND_MGR_GET_FLAGS_IPV6);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_ND_IPC_DELETEALLDYNAMICIPV4:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_DeleteAllDynamicIpNetToPhysicalEntry(ND_MGR_GET_FLAGS_IPV4);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_ND_IPC_SETTIMEOUT:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetIpNetToPhysicalTimeout(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

        case     NETCFG_MGR_ND_IPC_GETNEXTNDENTRY:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_GetNextIpNetToPhysicalEntry(ND_MGR_GET_FLAGS_IPV4 | ND_MGR_GET_FLAGS_IPV6,&(msg_p->data.arg_nd_entry));
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_nd_entry);
            break;
        case NETCFG_MGR_ND_IPC_GETNEXTIPV4NDENTRY:
            //msg_p->data.arg_nd_entry.ip_net_to_physical_net_address.type = L_INET_ADDR_TYPE_IPV4;
            msg_p->type.result_ui32 = NETCFG_MGR_ND_GetNextIpNetToPhysicalEntry(ND_MGR_GET_FLAGS_IPV4,&(msg_p->data.arg_nd_entry));
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_nd_entry);
            break;
        case NETCFG_MGR_ND_IPC_GETSTATISTICS:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_GetStatistics(&(msg_p->data.arg_nd_stat));
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_nd_stat);
            break;

        case NETCFG_MGR_ND_IPC_GETRUNNINGTIMEOUT:
            msg_p->type.result_running_cfg = NETCFG_MGR_ND_GetRunningIpNetToPhysicalTimeout(&(msg_p->data.ui32_v));
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(ui32_v);
            break;

        case NETCFG_MGR_ND_IPC_GETNEXTSTATICNDENTRY:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_GetNextStaticIpNetToPhysicalEntry(ND_MGR_GET_FLAGS_IPV4|ND_MGR_GET_FLAGS_IPV6,&(msg_p->data.arg_nd_static_entry));
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_nd_static_entry);
            break;
        case NETCFG_MGR_ND_IPC_GETNEXTSTATICIPV4NDENTRY:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_GetNextStaticIpNetToPhysicalEntry(ND_MGR_GET_FLAGS_IPV4,&(msg_p->data.arg_nd_static_entry));
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_nd_static_entry);
            break;
        case NETCFG_MGR_ND_IPC_GETNDENTRY:
            msg_p->type.result_bool = NETCFG_MGR_ND_GetIpNetToPhysicalEntry(&(msg_p->data.arg_nd_entry));
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_nd_entry);
            break;

        case NETCFG_MGR_ND_IPC_SETSTATICND:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetStaticIpNetToPhysicalEntry(&(msg_p->data.arg_static_entry_and_ui32.static_entry), msg_p->data.arg_static_entry_and_ui32.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
#if (SYS_CPNT_IPV6 == TRUE)
        case NETCFG_MGR_ND_IPC_DELETEALLDYNAMICIPV6:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_DeleteAllDynamicIpNetToPhysicalEntry(ND_MGR_GET_FLAGS_IPV6);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
        case     NETCFG_MGR_ND_IPC_GETNEXTIPV6NDENTRY:
            //msg_p->data.arg_nd_entry.ip_net_to_physical_net_address.type = L_INET_ADDR_TYPE_IPV6;
            msg_p->type.result_ui32 = NETCFG_MGR_ND_GetNextIpNetToPhysicalEntry(ND_MGR_GET_FLAGS_IPV6,&(msg_p->data.arg_nd_entry));
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_nd_entry);
            break;
        case NETCFG_MGR_ND_IPC_GETNEXTSTATICIPV6NDENTRY:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_GetNextStaticIpNetToPhysicalEntry(ND_MGR_GET_FLAGS_IPV6,&(msg_p->data.arg_nd_static_entry));
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_nd_static_entry);
            break;
        case NETCFG_MGR_ND_IPC_SETDADATTEMPTS:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetNdDADAttempts(msg_p->data.arg_ifindex_and_ui32.vid_ifindex,msg_p->data.arg_ifindex_and_ui32.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_ND_IPC_GETRUNNINGDADATTEMPTS:
            msg_p->type.result_running_cfg = NETCFG_MGR_ND_GetRunningDADAttempts(msg_p->data.arg_ifindex_and_ui32.vid_ifindex,&msg_p->data.arg_ifindex_and_ui32.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_ifindex_and_ui32);
            break;
        case NETCFG_MGR_ND_IPC_SETNDNSINTERVAL:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetNdNsInterval(msg_p->data.arg_ifindex_and_ui32.vid_ifindex,msg_p->data.arg_ifindex_and_ui32.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

         case NETCFG_MGR_ND_IPC_UNSETNDNSINTERVAL:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_UnsetNdNsInterval(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_ND_IPC_GETRUNNINGNDNSINTERVAL:
            msg_p->type.result_running_cfg = NETCFG_MGR_ND_GetRunningNdNsInterval(msg_p->data.arg_ifindex_and_ui32.vid_ifindex, &msg_p->data.arg_ifindex_and_ui32.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_ifindex_and_ui32);
            break;

        case NETCFG_MGR_ND_IPC_SETNDREACHABLETIME:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetNdReachableTime(msg_p->data.arg_ifindex_and_ui32.vid_ifindex, msg_p->data.arg_ifindex_and_ui32.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_ND_IPC_UNSETNDREACHABLETIME:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_UnsetNdReachableTime(msg_p->data.arg_ifindex_and_ui32.vid_ifindex);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_ND_IPC_GETRUNNINGNDREACHABLETIME:
            msg_p->type.result_running_cfg = NETCFG_MGR_ND_GetRunningNdReachableTime(msg_p->data.arg_ifindex_and_ui32.vid_ifindex, &msg_p->data.arg_ifindex_and_ui32.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_ifindex_and_ui32);
            break;

#if (SYS_CPNT_IPV6_ROUTING == TRUE)
        case NETCFG_MGR_ND_IPC_SETNDHOPLIMIT:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetNdHoplimit(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_ND_IPC_UNSETNDHOPLIMIT:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_UnsetNdHoplimit();
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_ND_IPC_GETRUNNINGNDHOPLIMIT:
            msg_p->type.result_running_cfg = NETCFG_MGR_ND_GetRunningNdHoplimit(&msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(ui32_v);
            break;
        case NETCFG_MGR_ND_IPC_SETNDPREFIX:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetNdPrefix(msg_p->data.arg_nd_prefix.vid_ifindex,
                                                                &msg_p->data.arg_nd_prefix.prefix,
                                                                msg_p->data.arg_nd_prefix.valid_lifetime,
                                                                msg_p->data.arg_nd_prefix.preferred_lifetime,
                                                                msg_p->data.arg_nd_prefix.enable_onlink,
                                                                msg_p->data.arg_nd_prefix.enable_autoconf);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_ND_IPC_UNSETNDPREFIX:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_UnSetNdPrefix(msg_p->data.arg_nd_prefix.vid_ifindex,
                                                                &msg_p->data.arg_nd_prefix.prefix);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break   ;
        case NETCFG_MGR_ND_IPC_GETRUNNINGNEXTNDPREFIX:
            msg_p->type.result_running_cfg = NETCFG_MGR_ND_GetRunningNextNdPrefix(msg_p->data.arg_nd_prefix.vid_ifindex,
                                                                &msg_p->data.arg_nd_prefix.prefix,
                                                                &msg_p->data.arg_nd_prefix.valid_lifetime,
                                                                &msg_p->data.arg_nd_prefix.preferred_lifetime,
                                                                &msg_p->data.arg_nd_prefix.enable_onlink,
                                                                &msg_p->data.arg_nd_prefix.enable_autoconf);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_nd_prefix);
            break;
        case NETCFG_MGR_ND_IPC_SETNDMANAGEDCONFIG:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetNdManagedConfigFlag(msg_p->data.arg_ifindex_and_bool.vid_ifindex, msg_p->data.arg_ifindex_and_bool.bool_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_ND_IPC_GETRUNNINGNDMANAGEDCONFIG:
            msg_p->type.result_running_cfg = NETCFG_MGR_ND_GetRunningNdManagedConfigFlag(msg_p->data.arg_ifindex_and_bool.vid_ifindex, &msg_p->data.arg_ifindex_and_bool.bool_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_ifindex_and_ui32);
            break;
        case NETCFG_MGR_ND_IPC_SETNDOTHERCONFIG:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetNdOtherConfigFlag(msg_p->data.arg_ifindex_and_bool.vid_ifindex, msg_p->data.arg_ifindex_and_bool.bool_v );
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_ND_IPC_GETRUNNINGNDOTHERCONFIG:
            msg_p->type.result_running_cfg = NETCFG_MGR_ND_GetRunningNdOtherConfigFlag(msg_p->data.arg_ifindex_and_bool.vid_ifindex, &msg_p->data.arg_ifindex_and_bool.bool_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_ifindex_and_ui32);
            break;

        case NETCFG_MGR_ND_IPC_SETNDRASUPPRESS:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetNdRaSuppress(msg_p->data.arg_ifindex_and_bool.vid_ifindex, msg_p->data.arg_ifindex_and_bool.bool_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_ND_IPC_GETRUNNINGNDRASUPPRESS:
            msg_p->type.result_running_cfg = NETCFG_MGR_ND_GetRunningNdRaSuppress(msg_p->data.arg_ifindex_and_bool.vid_ifindex, &msg_p->data.arg_ifindex_and_bool.bool_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_ifindex_and_ui32);
            break;
        case NETCFG_MGR_ND_IPC_SETNDRALIFETIME:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetNdRaLifetime(msg_p->data.arg_ifindex_and_ui32.vid_ifindex, msg_p->data.arg_ifindex_and_ui32.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_ND_IPC_UNSETNDRALIFETIME:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_UnsetNdRaLifetime(msg_p->data.arg_ifindex_and_ui32.vid_ifindex);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_ND_IPC_GETRUNNINGNDRALIFETIME:
            msg_p->type.result_running_cfg = NETCFG_MGR_ND_GetRunningNdRaLifetime(msg_p->data.arg_ifindex_and_ui32.vid_ifindex, &msg_p->data.arg_ifindex_and_ui32.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_ifindex_and_ui32);
            break;
        case NETCFG_MGR_ND_IPC_SETNDRAINTERVAL:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetNdRaInterval(msg_p->data.arg_ifindex_and_ui32x2.vid_ifindex,
                                                                    msg_p->data.arg_ifindex_and_ui32x2.ui32_1_v,
                                                                    msg_p->data.arg_ifindex_and_ui32x2.ui32_2_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_ND_IPC_UNSETNDRAINTERVAL:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_UnsetNdRaInterval(msg_p->data.arg_ifindex_and_ui32.vid_ifindex);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_ND_IPC_GETRUNNINGNDRAINTERVAL:
            msg_p->type.result_running_cfg = NETCFG_MGR_ND_GetRunningNdRaInterval(msg_p->data.arg_ifindex_and_ui32x2.vid_ifindex,
                                                                                  &msg_p->data.arg_ifindex_and_ui32x2.ui32_1_v,
                                                                                  &msg_p->data.arg_ifindex_and_ui32x2.ui32_2_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_ifindex_and_ui32x2);
            break;
        case NETCFG_MGR_ND_IPC_SETNDROUTERPREFERENCE:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_SetNdRouterPreference(msg_p->data.arg_ifindex_and_ui32.vid_ifindex, msg_p->data.arg_ifindex_and_ui32.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_ND_IPC_GETRUNNINGNDROUTERPREFERENCE:
            msg_p->type.result_running_cfg = NETCFG_MGR_ND_GetRunningNdRouterPreference(msg_p->data.arg_ifindex_and_ui32.vid_ifindex, &msg_p->data.arg_ifindex_and_ui32.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_ifindex_and_ui32);
            break;
#endif /* #if (SYS_CPNT_IPV6_ROUTING == TRUE) */
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

        case NETCFG_MGR_ND_IPC_ADDROUTERREDUNDANCYPROTOCOLENTRY:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_AddRouterRedundancyProtocolIpNetToPhysicalEntry(msg_p->data.arg_nd_config.vid_ifindex,
                                                        &msg_p->data.arg_nd_config.ip_addr,
                                                        msg_p->data.arg_nd_config.phy_addr.phy_address_len,
                                                        msg_p->data.arg_nd_config.phy_addr.phy_address_cctet_string);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_ND_IPC_DELETEROUTERREDUNDANCYPROTOCOLENTRY:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_DeleteRouterRedundancyProtocolIpNetToPhysicalEntry(msg_p->data.arg_nd_config.vid_ifindex,
                                                        &msg_p->data.arg_nd_config.ip_addr,
                                                        msg_p->data.arg_nd_config.phy_addr.phy_address_len,
                                                        msg_p->data.arg_nd_config.phy_addr.phy_address_cctet_string);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
        case NETCFG_MGR_ND_IPC_RAGUARD_GETPORTSTATUS:
            msg_p->type.result_bool = NETCFG_MGR_ND_RAGUARD_GetPortStatus(
                                        msg_p->data.arg_ifindex_and_bool.vid_ifindex,
                                        &msg_p->data.arg_ifindex_and_bool.bool_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_ifindex_and_bool);
            break;

        case NETCFG_MGR_ND_IPC_RAGUARD_GETNEXTPORTSTATUS:
            msg_p->type.result_bool = NETCFG_MGR_ND_RAGUARD_GetNextPortStatus(
                                        &msg_p->data.arg_ifindex_and_bool.vid_ifindex,
                                        &msg_p->data.arg_ifindex_and_bool.bool_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_ifindex_and_bool);
            break;

        case NETCFG_MGR_ND_IPC_RAGUARD_SETPORTSTATUS:
            msg_p->type.result_bool = NETCFG_MGR_ND_RAGUARD_SetPortStatus(
                                        msg_p->data.arg_ifindex_and_bool.vid_ifindex,
                                        msg_p->data.arg_ifindex_and_bool.bool_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_ND_IPC_RAGUARD_GETRUNNINGPORTSTATUS:
            msg_p->type.result_ui32 = NETCFG_MGR_ND_RAGUARD_GetRunningPortStatus(
                                        msg_p->data.arg_ifindex_and_bool.vid_ifindex,
                                        &msg_p->data.arg_ifindex_and_bool.bool_v);
            msgbuf_p->msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_ifindex_and_bool);
            break;
#endif

        default:
            DBGprintf("unknown message, something wrong??");
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.result_bool = FALSE;
            msgbuf_p->msg_size = NETCFG_MGR_ND_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
}

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
                                                   UI8_T *phy_addr)
{
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T    static_entry;
    NETCFG_TYPE_InetRifConfig_T                 rif_config;
    NETCFG_TYPE_L3_Interface_T                  intf;
    UI32_T                                      return_val;
    UI32_T                                      type = 0;
    int                                         i = 0;
    BOOL_T                                      is_add_amtrl3 = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if ((ip_addr == NULL))
        return NETCFG_TYPE_INVALID_ARG;

    if(vid_ifindex > (SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1))
        return NETCFG_TYPE_INVALID_ARG;

    if (  (L_INET_IS_IPV4_ADDR_TYPE(ip_addr->type))
        &&(IP_LIB_OK != IP_LIB_IsValidForRemoteIp(ip_addr->addr))
       )
        return NETCFG_TYPE_INVALID_ARG;

    if((phy_addr == NULL) || (phy_addr_len != NETCFG_TYPE_PHY_ADDRESEE_LENGTH))
    {
        if(phy_addr ==NULL)
            DBGprintf("input phy_addr = NULL");
        else
            DBGprintf("input phy_addr_len = %ld", (long)phy_addr_len);
        return NETCFG_TYPE_INVALID_ARG;
    }

    /* Invalid mac-address
     */
    if(!IS_VALID_MAC(phy_addr))
    {
        DBGprintf("Invalid zero/multicast/ broadcast mac-address.\r\n");
        return NETCFG_TYPE_INVALID_ARG;
    }

    if (NETCFG_MGR_ND_GetNDEntryType(ip_addr, &type))
    {
        if (type == VAL_ipNetToMediaExtType_vrrp)
        {
            DBGprintf("VRRP addr can not be configured!");
            return NETCFG_TYPE_IP_IS_VRRP_ADDRESS;
        }
    }

    memset(&intf, 0, sizeof(intf));
    memset(&rif_config, 0, sizeof(rif_config));
    rif_config.addr = *ip_addr;
    if(NETCFG_OM_IP_GetRifFromSubnet(&rif_config) == NETCFG_TYPE_OK)
    {
        /* Avoid configure static ARP on loopback interface
         */
        intf.ifindex = rif_config.ifindex;
        if (NETCFG_TYPE_OK != NETCFG_OM_IP_GetL3Interface(&intf))
            return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

        if (TRUE == IP_LIB_IsLoopbackInterface(intf.u.physical_intf.if_flags))
            return NETCFG_TYPE_CAN_NOT_ADD;
    }

    memset(&static_entry, 0, sizeof(static_entry));
    static_entry.ip_net_to_physical_entry.ip_net_to_physical_net_address = *ip_addr;

    if(NETCFG_OM_ND_GetStaticEntry(&static_entry) == TRUE)
    {
        if (phy_addr_len == static_entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_len &&
            0 == memcmp(static_entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string,phy_addr,phy_addr_len))
        {
            return NETCFG_TYPE_OK; /* return OK if identical */
        }
        else
        {
            if (static_entry.status == TRUE)
            {
                ND_MGR_DeleteStaticIpNetToPhysicalEntry(rif_config.ifindex, ip_addr);
                static_entry.status = FALSE;
                NETCFG_OM_ND_UpdateStaticEntry(&static_entry);
            }
        }
    }

    /* Add to AMTRL3 and Kernel only when:
     *    1. vid_ifindex matches the rif's ifindex or vid_ifindex is zero
     *    2. the rif's interface is UP
     *    3. can't conflict with the rif's address
     */
    static_entry.status = FALSE;
    if ((vid_ifindex == 0) && (rif_config.ifindex == 0))
    {
        /* # route -n
         * Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
         * 100.100.100.101 169.254.0.1     255.255.255.255 UGH   20     0        0 VLAN10
         * # arp -a
         * ? (169.254.0.1) at 08:00:27:ca:e8:e6 [ether] PERM on VLAN10
         *
         * TODO: for bgp unnumbered -> add to AMTRL3
         */
        L_INET_AddrIp_T nexthop;
        UI32_T          nh_ifidx, nh_owner;

        if (NETCFG_TYPE_OK == ROUTE_MGR_FindBestRoute(ip_addr, &nexthop, &nh_ifidx, &nh_owner))
        {
            if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&nexthop,
                                                               ip_string_buffer,
                                                               sizeof(ip_string_buffer)))
                sprintf((char *)ip_string_buffer,"UNKNOWNADDR");
            is_add_amtrl3 = TRUE;
            rif_config.ifindex = nh_ifidx;
            DBGprintf("%s:%d nh_idx/nh-%ld/%s\n", __FUNCTION__, __LINE__, nh_ifidx, ip_string_buffer);
        }
    }
    else
    {
        if( (((vid_ifindex == 0) && (rif_config.ifindex != 0)) ||
             ((vid_ifindex != 0) && (rif_config.ifindex == vid_ifindex))) &&
            (intf.u.physical_intf.if_flags & IFF_RUNNING) &&
            (0 != memcmp(ip_addr->addr, rif_config.addr.addr, ip_addr->addrlen)))
        {
            is_add_amtrl3 = TRUE;
        }
    }

    if (TRUE == is_add_amtrl3)
    {
        return_val = ND_MGR_AddStaticIpNetToPhysicalEntry(rif_config.ifindex,ip_addr,phy_addr_len,phy_addr);
        if(return_val != NETCFG_TYPE_OK)
        {
            DBGprintf("ND_MGR_AddStaticIpNetToPhysicalEntry fail: %lX", (unsigned long)return_val);
            return return_val;
        }
        static_entry.status = TRUE;
    }

    static_entry.ip_net_to_physical_entry.ip_net_to_physical_if_index = vid_ifindex;
    memcpy(static_entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string, phy_addr, phy_addr_len);
    static_entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_type = SN_LAN;
    static_entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_len = phy_addr_len;
    static_entry.ip_net_to_physical_entry.ip_net_to_physical_type = VAL_ipNetToMediaType_static;

    if(NETCFG_OM_ND_AddStaticEntry(&static_entry) == TRUE)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if (static_entry.status == TRUE)
        {
            ND_MGR_DeleteStaticIpNetToPhysicalEntry(rif_config.ifindex, ip_addr);
            if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)ip_addr,
                                                               ip_string_buffer,
                                                               sizeof(ip_string_buffer)))
                sprintf((char *)ip_string_buffer,"UNKNOWNADDR");
            DBGprintf("Can not add to OM, rollback ifidx=%ld, addr=%s", (long)rif_config.ifindex,ip_string_buffer);
        }

        return NETCFG_TYPE_CAN_NOT_ADD;
    }
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_DeleteStaticIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete a static ND entry.
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
UI32_T NETCFG_MGR_ND_DeleteStaticIpNetToPhysicalEntry(UI32_T vid_ifindex, L_INET_AddrIp_T* ip_addr)
{
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T    entry;
    NETCFG_TYPE_InetRifConfig_T                 rif_config;
    UI32_T                                      tmp_if_index = 0;
    BOOL_T                                      rc;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if (ip_addr == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    if (vid_ifindex > (SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1))
    {
        DBGprintf("invlaid ifindex: %ld", (long)vid_ifindex);
        return NETCFG_TYPE_INVALID_ARG;
    }

    memset(&entry,0,sizeof(entry));
    /* entry.ip_net_to_physical_entry.ip_net_to_physical_if_index = vid_ifindex; */
    entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.type = ip_addr->type;
    memcpy(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr ,ip_addr->addr, ip_addr->addrlen);

    if(NETCFG_OM_ND_GetStaticEntry(&entry) == FALSE)
    {
        DBGprintf("can not find entry in OM");
        return NETCFG_TYPE_OK; /* delete non-exist entry treated as success */
    }

    if (entry.status == TRUE)
    {
        if (entry.ip_net_to_physical_entry.ip_net_to_physical_if_index != 0)
        {
            tmp_if_index = entry.ip_net_to_physical_entry.ip_net_to_physical_if_index;
        }
        else
        {
            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.addr = *ip_addr;
            if (NETCFG_OM_IP_GetRifFromSubnet(&rif_config) == NETCFG_TYPE_OK)
            {
                tmp_if_index = rif_config.ifindex;
            }
            else
            {
                L_INET_AddrIp_T nexthop;
                UI32_T          nh_ifidx, nh_owner;

                /* # route -n
                 * Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
                 * 100.100.100.101 169.254.0.1     255.255.255.255 UGH   20     0        0 VLAN10
                 * # arp -a
                 * ? (169.254.0.1) at 08:00:27:ca:e8:e6 [ether] PERM on VLAN10
                 *
                 * TODO: for bgp unnumbered case
                 */
                if (NETCFG_TYPE_OK == ROUTE_MGR_FindBestRoute(ip_addr, &nexthop, &nh_ifidx, &nh_owner))
                {
                    if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&nexthop,
                                                                       ip_string_buffer,
                                                                       sizeof(ip_string_buffer)))
                        sprintf((char *)ip_string_buffer,"UNKNOWNADDR");
                    tmp_if_index = nh_ifidx;
                    DBGprintf("%s:%d nh_idx/nh-%ld/%s\n", __FUNCTION__, __LINE__, nh_ifidx, ip_string_buffer);
                }
                else
                {
                    return NETCFG_TYPE_CAN_NOT_DELETE;
                }
            }
        }

        ND_MGR_DeleteStaticIpNetToPhysicalEntry(tmp_if_index, ip_addr);
    }

    rc = NETCFG_OM_ND_DeleteStaticEntry(&entry);
    if(rc == FALSE)
    {
        if (entry.status == TRUE)
        {
            ND_MGR_AddStaticIpNetToPhysicalEntry(tmp_if_index,
                                                 ip_addr,
                                                 entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_len,
                                                 entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string);
        }

        if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)ip_addr,
                                                           ip_string_buffer,
                                                           sizeof(ip_string_buffer)))
            sprintf((char *)ip_string_buffer,"UNKNOWNADDR");
        DBGprintf("Can not delete OM, rollback ifidx=%ld,addr=%s", (long)vid_ifindex,ip_string_buffer);

        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_GetNextStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE :Lookup next static ND entry.
 *
 * INPUT   : entry.
 *               action_flags: possible action_flags are ND_MGR_GET_FLAGS_IPV4 and ND_MGR_GET_FLAGS_IPV6
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */

UI32_T NETCFG_MGR_ND_GetNextStaticIpNetToPhysicalEntry(UI32_T action_flags, NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry)
{
    BOOL_T result;
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
    /*check end*/

    if(action_flags & ND_MGR_GET_FLAGS_IPV4 && action_flags & ND_MGR_GET_FLAGS_IPV6 )
    {
        /*1st get V4, then get V6*/
        result = FALSE;
        if(entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type<=L_INET_ADDR_TYPE_IPV4)//V4 or 0=get 1st
            result = NETCFG_OM_ND_GetNextStaticEntry(L_INET_ADDR_TYPE_IPV4,entry);
        if(result != TRUE)
            result = NETCFG_OM_ND_GetNextStaticEntry(L_INET_ADDR_TYPE_IPV6,entry);
    }
    else if (action_flags & ND_MGR_GET_FLAGS_IPV4)
        result = NETCFG_OM_ND_GetNextStaticEntry(L_INET_ADDR_TYPE_IPV4,entry);
    else if (action_flags & ND_MGR_GET_FLAGS_IPV6)
        result = NETCFG_OM_ND_GetNextStaticEntry(L_INET_ADDR_TYPE_IPV6,entry);
    else
    {
        DBGprintf("address type");
        return NETCFG_TYPE_FAIL;
    }


    if ( TRUE == result)
        return NETCFG_TYPE_OK;
    else
    {
        return NETCFG_TYPE_FAIL;
    }
}

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
UI32_T NETCFG_MGR_ND_SetIpNetToPhysicalTimeout(UI32_T age_time)
{
    UI32_T      ret;
    UI32_T      pre_timeout;

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(age_time < SYS_ADPT_MIN_ARP_CACHE_TIMEOUT || age_time > SYS_ADPT_MAX_ARP_CACHE_TIMEOUT)
    {
        DBGprintf("invalid age_time: %ld", (long)age_time);
        return NETCFG_TYPE_INVALID_ARG;
    }
    /*check end*/

    NETCFG_OM_ND_GetTimeout(&pre_timeout);

    ret = ND_MGR_SetIpNetToMediaEntryTimeout(age_time,pre_timeout);

    if (ret != NETCFG_TYPE_OK)
    {
        DBGprintf("Fail to call ND: %lx", (unsigned long)ret);
        return ret;
    }

    if(NETCFG_OM_ND_SetTimeout(age_time) == FALSE)
    {
        ND_MGR_SetIpNetToMediaEntryTimeout(pre_timeout,pre_timeout);
        DBGprintf("fail to cll OM, roll back %ld->%ld", (long)age_time, (long)pre_timeout);
        return NETCFG_TYPE_CAN_NOT_SET;
    }

    return NETCFG_TYPE_OK;
}

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
                                                                     UI8_T *phy_addr)
{
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T        static_entry;
    NETCFG_OM_ND_RouterRedundancyEntry_T rp_entry;

    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if (NULL == ip_addr)
        return NETCFG_TYPE_INVALID_ARG;

    if ((vid_ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER) ||
        (vid_ifindex > SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    if((phy_addr == NULL) || (phy_addr_len != NETCFG_TYPE_PHY_ADDRESEE_LENGTH))
    {
        if(phy_addr ==NULL)
            DBGprintf("input phy_addr = NULL");
        else
            DBGprintf("input phy_addr_len = %ld", (long)phy_addr_len);
        return NETCFG_TYPE_INVALID_ARG;
    }

    memset(&static_entry, 0, sizeof(static_entry));
    static_entry.ip_net_to_physical_entry.ip_net_to_physical_net_address = *ip_addr;

    ret = NETCFG_OM_ND_GetStaticEntry(&static_entry);
    /* If there's a static ARP entry that the ip address equals VRRP virtual IP.
     * Delete the static ARP entry.
     */
    if (TRUE == ret)
        ND_MGR_DeleteStaticIpNetToPhysicalEntry(vid_ifindex, ip_addr);

    memset(&rp_entry, 0, sizeof(rp_entry));
    rp_entry.ifindex = vid_ifindex;
    rp_entry.ip_addr = *ip_addr;
    memcpy(rp_entry.mac, phy_addr, SYS_ADPT_MAC_ADDR_LEN);
    if(NETCFG_OM_ND_AddRouterRedundancyProtocolIpNetToPhysicalEntry(&rp_entry) == FALSE)
        return NETCFG_TYPE_FAIL;

    return NETCFG_TYPE_OK;
}

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
                                                                         UI8_T *phy_addr)
{
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T        static_entry;
    NETCFG_OM_ND_RouterRedundancyEntry_T rp_entry;
    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if (NULL == ip_addr)
        return NETCFG_TYPE_INVALID_ARG;

    if ((vid_ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER) ||
        (vid_ifindex > SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    if((phy_addr == NULL) || (phy_addr_len != NETCFG_TYPE_PHY_ADDRESEE_LENGTH))
    {
        if(phy_addr ==NULL)
            DBGprintf("input phy_addr = NULL");
        else
            DBGprintf("input phy_addr_len = %ld", (long)phy_addr_len);
        return NETCFG_TYPE_INVALID_ARG;
    }

    memset(&static_entry, 0, sizeof(static_entry));
    static_entry.ip_net_to_physical_entry.ip_net_to_physical_net_address = *ip_addr;

    ret = NETCFG_OM_ND_GetStaticEntry(&static_entry);
    /* If there's a static ARP entry that the ip address equals VRRP virtual IP.
     * Delete the static ARP entry.
     */
    if (TRUE == ret)
    {
        ND_MGR_AddStaticIpNetToPhysicalEntry(
            vid_ifindex,
            ip_addr,
            phy_addr_len,
            static_entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string);
    }

    memset(&rp_entry, 0, sizeof(rp_entry));
    rp_entry.ifindex = vid_ifindex;
    rp_entry.ip_addr = *ip_addr;
    memcpy(rp_entry.mac, phy_addr, SYS_ADPT_MAC_ADDR_LEN);
    if(NETCFG_OM_ND_DeleteRouterRedundancyProtocolIpNetToPhysicalEntry(&rp_entry) == FALSE)
        return NETCFG_TYPE_FAIL;

    return NETCFG_TYPE_OK;
}

#if (SYS_CPNT_IPV6 == TRUE)
UI32_T NETCFG_MGR_ND_SetIpv6NetToPhysicalTimeout(UI32_T age_time)
{
    UI32_T      ret;
    UI32_T      pre_timeout;

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(age_time < SYS_ADPT_MIN_ND_CACHE_TIMEOUT || age_time > SYS_ADPT_MAX_ND_CACHE_TIMEOUT)
    {
        DBGprintf("INvalid age_time %ld", (long)age_time);
        return NETCFG_TYPE_INVALID_ARG;
    }
    /*check end*/

    NETCFG_OM_ND_GetTimeout(&pre_timeout);

    ret = ND_MGR_SetIpNetToMediaEntryTimeout(age_time,pre_timeout);

    if (ret != NETCFG_TYPE_OK)
    {
        return ret;
    }

    if(NETCFG_OM_ND_SetTimeout(age_time) == FALSE)
    {
        ND_MGR_SetIpNetToMediaEntryTimeout(pre_timeout,pre_timeout);
        DBGprintf("Fail to set OM, rollback %ld->%ld", (long)age_time, (long)pre_timeout);
        return NETCFG_TYPE_CAN_NOT_SET;
    }

    return NETCFG_TYPE_OK;
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

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
UI32_T NETCFG_MGR_ND_DeleteAllDynamicIpNetToPhysicalEntry(UI32_T actionflags)
{
    UI32_T      ret;

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
    /*check end*/

    ret = ND_MGR_DeleteAllDynamicIpNetToPhysicalEntry(actionflags);
    //no OM to delete
    return ret;
}

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
UI32_T NETCFG_MGR_ND_GetNextIpNetToPhysicalEntry(UI32_T action_flags, NETCFG_TYPE_IpNetToPhysicalEntry_T *entry)
{
    UI32_T      ret;

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
    /*check end*/

    ret = ND_MGR_GetNextIpNetToPhysicalEntry(action_flags, entry);

    return ret;
}

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
UI32_T NETCFG_MGR_ND_GetStatistics(NETCFG_TYPE_IpNetToMedia_Statistics_T *stat)
{
    UI32_T        ret;

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
    /*check end*/

    ret = ND_MGR_GetStatistic(stat);

    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_GetRunningIpNetToMediaTimeout
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
NETCFG_MGR_ND_GetRunningIpNetToPhysicalTimeout(UI32_T *age_time)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    /*check end*/

    if ( TRUE == NETCFG_OM_ND_GetTimeout(age_time))
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
 * ROUTINE NAME : NETCFG_MGR_ND_GetIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Get an ND entry information.
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
BOOL_T NETCFG_MGR_ND_GetIpNetToPhysicalEntry(NETCFG_TYPE_IpNetToPhysicalEntry_T *entry)
{
    UI32_T        ret;

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;
    /*check end*/

    ret = ND_MGR_GetIpNetToPhysicalEntry(entry);

    if (ret != NETCFG_TYPE_OK)
        return FALSE;
    else
        return TRUE;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_SetStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Set a static ND entry invalid or valid.
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
UI32_T NETCFG_MGR_ND_SetStaticIpNetToPhysicalEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry, int type)
{
    UI32_T        ret;
    L_INET_AddrIp_T inet_addr;
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    inet_addr =  NETCFG_MGR_COMPOSEINETADDR(entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.type,
                                    entry->ip_net_to_physical_entry.ip_net_to_physical_net_address.addr,0);
    switch(type)
    {
        case  VAL_ipNetToMediaType_invalid :
            ret = NETCFG_MGR_ND_DeleteStaticIpNetToPhysicalEntry(entry->ip_net_to_physical_entry.ip_net_to_physical_if_index, &inet_addr);
        break;
        case  VAL_ipNetToMediaType_static :
            ret = NETCFG_MGR_ND_AddStaticIpNetToPhysicalEntry(entry->ip_net_to_physical_entry.ip_net_to_physical_if_index,
                                                            &inet_addr,
                                                            entry->ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_len,
                                                            entry->ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string);
        break;
        case  VAL_ipNetToMediaType_dynamic :
        case  VAL_ipNetToMediaType_other :
        default :
            ret = NETCFG_TYPE_NOT_IMPLEMENT;
        break;
    }
    return ret;
}

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
UI32_T NETCFG_MGR_ND_SignalL3IfUp(UI32_T ifindex)
{
#if (SYS_CPNT_IPV6 == TRUE)
    /* config ND config into kernel.
     */
    UI32_T         rc;
    UI32_T attempts, msec, hoplimit;
    BOOL_T is_set = FALSE;

    if(!NETCFG_OM_ND_GetNdDADAttempts(ifindex, &attempts))
        attempts = SYS_DFLT_ND_DUPLICATE_ADDRESS_DETECTION_ATTEMPTS;

    rc = ND_MGR_SetNsDadAttempts(ifindex, attempts);

    /* ns-interval */
    NETCFG_OM_ND_IsConfigFlagSet(ifindex,NETCFG_OM_ND_FLAG_ISSET_NS_INTERVAL, &is_set);
    if(is_set)
    {
        NETCFG_OM_ND_GetNdNsInterval(ifindex, &msec);
        IPAL_ICMP_SetIpv6NeighRetransTime(ifindex, msec);
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
        NSM_PMGR_SetNsInterval(ifindex, msec);
#endif
    }
    else
    {
        msec = SYS_DFLT_ND_NEIGHBOR_SOLICITATION_RETRANSMISSIONS_INTERVAL;
        IPAL_ICMP_SetIpv6NeighRetransTime(ifindex, msec);
    }

    /* reachable time */
    NETCFG_OM_ND_IsConfigFlagSet(ifindex, NETCFG_OM_ND_FLAG_ISSET_REACHABLE_TIME, &is_set);
    if(is_set)
    {
        if(TRUE == NETCFG_OM_ND_GetNdReachableTime(ifindex, &msec))
            ND_MGR_SetNdReachableTime(ifindex, msec);

    }
    else
    {
        ND_MGR_UnsetNdReachableTime(ifindex);
    }

    /* hop-limit */
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    NETCFG_OM_ND_IsConfigFlagSet(ifindex, NETCFG_OM_ND_FLAG_ISSET_RA_HOPLIMIT, &is_set);
    if(is_set)
    {
        if(TRUE == NETCFG_OM_ND_GetNdHoplimit(ifindex, &hoplimit))
            ND_MGR_SetHopLimit(hoplimit);
    }
    else
    {
        ND_MGR_UnsetHopLimit();
    }
#endif
#endif
    return NETCFG_TYPE_OK;
}

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
UI32_T NETCFG_MGR_ND_SignalRifUp(UI32_T ifindex, L_INET_AddrIp_T* ip_addr)
{
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T   entry;
    UI32_T         rc;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    memset(&entry, 0, sizeof(entry));
    while(NETCFG_OM_ND_GetNextStaticEntry(ip_addr->type, &entry) == TRUE)
    {
        if((entry.ip_net_to_physical_entry.ip_net_to_physical_if_index == 0 ||
            ifindex == entry.ip_net_to_physical_entry.ip_net_to_physical_if_index) &&
           (0 == NETCFG_MGR_PREFIX_COMPARE(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr, ip_addr->addr, ip_addr->preflen)))
        {
            /* if Static ARP entry ip address is same as local RIF
             */
            if(0 == memcmp(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr, ip_addr->addr, ip_addr->addrlen))
            {
                if (TRUE == entry.status)
                {
                    ND_MGR_DeleteStaticIpNetToPhysicalEntry(ifindex,&entry.ip_net_to_physical_entry.ip_net_to_physical_net_address);

                    entry.status = FALSE;
                    NETCFG_OM_ND_UpdateStaticEntry(&entry);
                }
            }
            else if(FALSE == entry.status)
            {
                if (NETCFG_TYPE_OK == ND_MGR_AddStaticIpNetToPhysicalEntry(ifindex,
                                                        &entry.ip_net_to_physical_entry.ip_net_to_physical_net_address,
                                                        entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_len,
                                                        entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string))
                {
                    entry.status = TRUE;
                    NETCFG_OM_ND_UpdateStaticEntry(&entry);
                }
            }
        }
    }

#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    if(ip_addr->type == L_INET_ADDR_TYPE_IPV6Z)
    {
        BOOL_T ra_suppress, ra_enable;

        if(!NETCFG_OM_ND_GetNdRaSuppress(ifindex, &ra_suppress))
            ra_suppress = SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_SUPPRESS;
        ra_enable = !ra_suppress;
        /* config to NSM */
        rc = NSM_PMGR_SetRa(ifindex, ra_enable);
    }
#endif

/* for removal warning */
    return NETCFG_TYPE_OK;
}

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
UI32_T NETCFG_MGR_ND_SignalRifDown(UI32_T ifindex, L_INET_AddrIp_T* ip_addr)
{
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T    entry;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    memset(&entry, 0, sizeof(entry));
    while(NETCFG_OM_ND_GetNextStaticEntry(ip_addr->type, &entry) == TRUE)
    {
        if(0 == NETCFG_MGR_PREFIX_COMPARE(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr, ip_addr->addr, ip_addr->preflen))
        {
            if(entry.status == TRUE)
            {
                ND_MGR_DeleteStaticIpNetToPhysicalEntry(ifindex, &entry.ip_net_to_physical_entry.ip_net_to_physical_net_address);

                entry.status = FALSE;
                NETCFG_OM_ND_UpdateStaticEntry(&entry);
            }
        }
    }

#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    /* In RFC4861, sec 6.2.8.  Link-local Address Change
     * The overall effect should be the same as if one interface ceases being an
     * advertising interface, and a different one starts being an advertising interface
     * The sending of RA with lifetime 0 is triggered by rif down instead of rif destroy
     * because when rif destroy happen, the rif is already down and can't sent out RA packet
     */
    if (ip_addr->type == L_INET_ADDR_TYPE_IPV6Z)
    {
        if (NSM_TYPE_RESULT_OK != NSM_PMGR_SetRa(ifindex, FALSE))
            return NETCFG_TYPE_FAIL;
    }
#endif

    return NETCFG_TYPE_OK;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ND_SignalRifCreate
 *-----------------------------------------------------------------------------
 * PURPOSE : when create a rif signal ND .
 *
 * INPUT   : ip_addr, ip_mask.
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK
 *           NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ND_SignalRifCreate(L_INET_AddrIp_T* ip_addr)
{//Simon: just porting, don't seems to be correct!
#if 0
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T     entry,temp_entry;
    NETCFG_OM_IP_InetRifConfig_T              rif;
    UI32_T                      rc;
    UI32_T                      ifindex;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    NOTEprintf("input=%s,addr[%d]=%X:%X:%X:%X:%X:%X:%X:%X", (ip_addr->type ==L_INET_ADDR_TYPE_IPV4)?"IPv4":"IPv6", ip_addr->addrlen,L_INET_EXPAND_IPV6(ip_addr->addr));

    memset(&rif,0, sizeof(rif));
    rif.addr = * ip_addr;
    if(NETCFG_OM_IP_GetInetRif(&rif) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_INVALID_IP;

    ifindex = rif.ifindex;
    memset(&rif.addr.addr, 0, ip_addr->addrlen);

    memset(&entry,0,sizeof(entry));
    while(NETCFG_OM_ND_GetNextStaticEntry(ip_addr->type, &entry) == TRUE)
    {
        if(0 == NETCFG_MGR_PREFIX_COMPARE(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr, ip_addr->addr, ip_addr->preflen))
        {
            if(0 == memcmp(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr, ip_addr->addr, ip_addr->addrlen))
            {//static entry --> link local
                rc = ND_MGR_DeleteStaticIpNetToPhysicalEntry(entry.ip_net_to_physical_entry.ip_net_to_physical_if_index,&entry.ip_net_to_physical_entry.ip_net_to_physical_net_address);
                if(rc != NETCFG_TYPE_OK)
                    continue;
                else
                {
                    temp_entry = entry;
                    NETCFG_OM_ND_DeleteStaticEntry(&temp_entry);
                    temp_entry.status = FALSE;
                    temp_entry.ip_net_to_physical_entry.ip_net_to_physical_if_index =0;
                    NETCFG_OM_ND_AddStaticEntry(&temp_entry);
                }
            }
            else if(FALSE == entry.status)
            {//static entry --> status TRUE
                rc = ND_MGR_AddStaticIpNetToPhysicalEntry(
                                                        ifindex,
                                                        &entry.ip_net_to_physical_entry.ip_net_to_physical_net_address,
                                                        entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_len,
                                                        entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string);
                if(rc != NETCFG_TYPE_OK)
                    continue;
                else
                {
                    temp_entry = entry;
                    NETCFG_OM_ND_DeleteStaticEntry(&temp_entry);
                    temp_entry.status = TRUE;
                    temp_entry.ip_net_to_physical_entry.ip_net_to_physical_if_index =ifindex;
                    NETCFG_OM_ND_AddStaticEntry(&temp_entry);
                }
            }
        }
    }
#endif

    return NETCFG_TYPE_OK;
}

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
UI32_T NETCFG_MGR_ND_SignalRifDestroy(L_INET_AddrIp_T* ip_addr)
{//Simon: just porting, don't seems to be correct!
#if 0
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T      entry,temp_entry;
    NETCFG_OM_IP_InetRifConfig_T                rif;
    NETCFG_TYPE_InetRifConfig_T                  rifconfig;

    UI32_T ifindex;
    UI32_T                    rc;
    BOOL_T                    if_network_has_other_addr = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    memset(&rif, 0, sizeof(rif));

    rif.addr = *ip_addr;
    if(NETCFG_OM_IP_GetInetRif(&rif) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_INVALID_IP;

    ifindex = rif.ifindex;
    memset(&rifconfig, 0, sizeof(rifconfig));
    rifconfig.addr = * ip_addr;
    if(NETCFG_OM_IP_GetRifFromSubnet(&rifconfig) == NETCFG_TYPE_OK)
        if_network_has_other_addr = TRUE;

    while(NETCFG_OM_ND_GetNextStaticEntry(ip_addr->type, &entry) == TRUE)
    {
        if(0 == NETCFG_MGR_PREFIX_COMPARE(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr, ip_addr->addr, ip_addr->preflen))
        {
            if(0 == memcmp(entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr ,ip_addr->addr, ip_addr->addrlen))
            {//link local -->static
                rc = ND_MGR_AddStaticIpNetToPhysicalEntry(
                                                        ifindex,
                                                        &entry.ip_net_to_physical_entry.ip_net_to_physical_net_address,
                                                        entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_len,
                                                        entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string);

                if(rc != NETCFG_TYPE_OK)
                    continue;
                else
                {
                    temp_entry = entry;
                    NETCFG_OM_ND_DeleteStaticEntry(&temp_entry);
                    temp_entry.status = TRUE;
                    temp_entry.ip_net_to_physical_entry.ip_net_to_physical_if_index =ifindex;
                    NETCFG_OM_ND_AddStaticEntry(&temp_entry);
                }
            }
            else if(if_network_has_other_addr == FALSE)
            { //static -> status false
                rc = ND_MGR_DeleteStaticIpNetToPhysicalEntry(entry.ip_net_to_physical_entry.ip_net_to_physical_if_index, &entry.ip_net_to_physical_entry.ip_net_to_physical_net_address);
                if(rc != NETCFG_TYPE_OK)
                    continue;
                else
                {
                    temp_entry = entry;
                    NETCFG_OM_ND_DeleteStaticEntry(&temp_entry);
                    temp_entry.status = FALSE;
                    temp_entry.ip_net_to_physical_entry.ip_net_to_physical_if_index =0;
                    NETCFG_OM_ND_AddStaticEntry(&temp_entry);
                }
            }
        }
    }
#endif

    return NETCFG_TYPE_OK;
}

#if (SYS_CPNT_IPV6 == TRUE)
UI32_T NETCFG_MGR_ND_SetNdDADAttempts(UI32_T vid_ifindex, UI32_T attempts)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(NETCFG_OM_ND_SetNdDADAttempts(vid_ifindex, attempts))
    {
        ND_MGR_SetNsDadAttempts(vid_ifindex, attempts);
    }

    return NETCFG_TYPE_OK;
}

SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningDADAttempts(UI32_T vid_ifindex, UI32_T* attempts)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    /*check end*/

    if ( TRUE == NETCFG_OM_ND_GetNdDADAttempts(vid_ifindex, attempts))
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
UI32_T NETCFG_MGR_ND_SetNdNsInterval(UI32_T vid_ifindex, UI32_T msec)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(NETCFG_OM_ND_SetNdNsInterval(vid_ifindex,msec))
    {
        ND_MGR_SetNsInterval(vid_ifindex, msec);
    }

    return NETCFG_TYPE_OK;
}

UI32_T NETCFG_MGR_ND_UnsetNdNsInterval(UI32_T vid_ifindex)
{

    UI32_T      pre_msec;//for rollback


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(!NETCFG_OM_ND_GetNdNsInterval(vid_ifindex, &pre_msec))
        return NETCFG_TYPE_FAIL;

    //step1: write to ND_MGR
    ND_MGR_UnsetNsInterval(vid_ifindex);

    //step2: write to OM
    if(!NETCFG_OM_ND_UnsetNdNsInterval(vid_ifindex ))
    {
        DBGprintf("Fail to call OM(%ld),rollback to %ld", (long)vid_ifindex, (long)pre_msec);
        ND_MGR_SetNsInterval(vid_ifindex,pre_msec );
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}


SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdNsInterval(UI32_T vid_ifindex, UI32_T* msec)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    /*check end*/

    if(TRUE == NETCFG_OM_ND_IsSetNdNsInterval(vid_ifindex))
    {
        if ( TRUE == NETCFG_OM_ND_GetNdNsInterval( vid_ifindex,msec))
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

UI32_T NETCFG_MGR_ND_SetNdReachableTime(UI32_T vid_ifindex,UI32_T msec)
{
    UI32_T      pre_msec;//for rollback

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    //if(pre_msec < low_bound || pre_msec > high_bound)
    //    return NETCFG_TYPE_INVALID_ARG;
    /*check end*/

    if(!NETCFG_OM_ND_GetNdReachableTime(vid_ifindex,&pre_msec))
        return NETCFG_TYPE_FAIL;

    //step1: write to ND_MGR
    if(NETCFG_TYPE_OK != ND_MGR_SetNdReachableTime( vid_ifindex, msec))
        return NETCFG_TYPE_FAIL;
    //step2: write to OM
    if(!NETCFG_OM_ND_SetNdReachableTime( vid_ifindex, msec))
    {
        ND_MGR_SetNdReachableTime( vid_ifindex, pre_msec);
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}
UI32_T NETCFG_MGR_ND_UnsetNdReachableTime(UI32_T vid_ifindex)
{
    UI32_T      pre_msec;//for rollback

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(!NETCFG_OM_ND_GetNdReachableTime(vid_ifindex,&pre_msec))
        return NETCFG_TYPE_FAIL;

    //step1: write to ND_MGR
    if(NETCFG_TYPE_OK != ND_MGR_UnsetNdReachableTime( vid_ifindex))
        return NETCFG_TYPE_FAIL;

    //step2: write to OM
    if(!NETCFG_OM_ND_UnsetNdReachableTime(vid_ifindex))
    {
        ND_MGR_SetNdReachableTime( vid_ifindex, pre_msec);
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdReachableTime(UI32_T vid_ifindex, UI32_T* msec)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    /*check end*/

    if( TRUE == NETCFG_OM_ND_IsSetNdReachableTime(vid_ifindex))
    {
        if ( TRUE == NETCFG_OM_ND_GetNdReachableTime( vid_ifindex, msec))
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

#if (SYS_CPNT_IPV6_ROUTING == TRUE)

UI32_T NETCFG_MGR_ND_SetNdHoplimit(UI32_T hoplimit)
{
    UI32_T      pre_hoplimit;//for rollback
    UI32_T      ret;

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    //if(pre_msec < low_bound || pre_msec > high_bound)
    //    return NETCFG_TYPE_INVALID_ARG;
    /*check end*/

    if(!NETCFG_OM_ND_GetGlobalNdHoplimit(&pre_hoplimit))
        return NETCFG_TYPE_FAIL;

    //step1: write to ND_MGR
    ret = ND_MGR_SetHopLimit(hoplimit );
    if(NETCFG_TYPE_OK != ret)
        return ret;

    //step2: write to OM
    if(!NETCFG_OM_ND_SetGlobalNdHoplimit( hoplimit ))
    {
        ND_MGR_SetHopLimit(pre_hoplimit );
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}
UI32_T NETCFG_MGR_ND_UnsetNdHoplimit()
{
    UI32_T      pre_hoplimit;//for rollback
    UI32_T      ret;
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(!NETCFG_OM_ND_GetGlobalNdHoplimit(&pre_hoplimit))
        return NETCFG_TYPE_FAIL;

    //step1: write to ND_MGR
    ret = ND_MGR_UnsetHopLimit();
    if(NETCFG_TYPE_OK != ret)
        return ret;
    //step2: write to OM
    if(!NETCFG_OM_ND_UnsetGlobalNdHoplimit())
    {
        ND_MGR_SetHopLimit(pre_hoplimit );
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdHoplimit( UI32_T* hoplimit)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    /*check end*/

    if( TRUE == NETCFG_OM_ND_IsSetGlobalNdHoplimit())
    {
        if ( TRUE == NETCFG_OM_ND_GetGlobalNdHoplimit( hoplimit))
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
        {
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

}
UI32_T NETCFG_MGR_ND_SetNdPrefix(UI32_T vid_ifindex, L_INET_AddrIp_T* prefix, UI32_T valid_lifetime, UI32_T preferred_lifetime,BOOL_T enable_onlink,BOOL_T enable_autoconf)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
    /*check end*/

    //step1: write to ND_MGR

    if(NSM_TYPE_RESULT_OK != NSM_PMGR_SetRaPrefix(vid_ifindex, prefix, valid_lifetime, preferred_lifetime, enable_onlink, enable_autoconf))
        return NETCFG_TYPE_FAIL;
    //step2: write to OM
    if(!NETCFG_OM_ND_SetNdPrefix(vid_ifindex, prefix, valid_lifetime, preferred_lifetime, enable_onlink, enable_autoconf))
    {
        NSM_PMGR_UnsetRaPrefix(vid_ifindex, prefix);
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}

UI32_T NETCFG_MGR_ND_UnSetNdPrefix(UI32_T vid_ifindex, L_INET_AddrIp_T* prefix)
{
    UI32_T pre_valid_lifetime,pre_preferred_lifetime;
    BOOL_T  pre_onLink, pre_autoconfig;

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
    /*check end*/

    if(!NETCFG_OM_ND_GetNdPrefix(vid_ifindex, prefix, &pre_valid_lifetime,&pre_preferred_lifetime,&pre_onLink, &pre_autoconfig))
        return NETCFG_TYPE_FAIL;

    //step1: write to ND_MGR
    //ND_MGR has nothing to do with preifx, so call NSM directly
    if(NSM_TYPE_RESULT_OK != NSM_PMGR_UnsetRaPrefix(vid_ifindex, prefix))
        return NETCFG_TYPE_FAIL;
    //step2: write to OM
    if(!NETCFG_OM_ND_UnsetNdPrefix(vid_ifindex, prefix))
    {
        NSM_PMGR_SetRaPrefix(vid_ifindex, prefix,pre_valid_lifetime,pre_preferred_lifetime,pre_onLink, pre_autoconfig   );
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}

SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNextNdPrefix(UI32_T vid_ifindex, L_INET_AddrIp_T* prefix, UI32_T *valid_lifetime, UI32_T *preferred_lifetime,BOOL_T*enable_on_link,BOOL_T* enable_autoconf)
{
    if(TRUE == NETCFG_OM_ND_GetNextNdPrefix(vid_ifindex, prefix, valid_lifetime,  preferred_lifetime,  enable_on_link,  enable_autoconf))
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

UI32_T NETCFG_MGR_ND_SetNdManagedConfigFlag(UI32_T vid_ifindex, BOOL_T enableFlag)
{
    BOOL_T      pre_enableFlag;//for rollback

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    //if(pre_msec < low_bound || pre_msec > high_bound)
    //    return NETCFG_TYPE_INVALID_ARG;
    /*check end*/



    if(!NETCFG_OM_ND_GetNdManagedConfigFlag(vid_ifindex,&pre_enableFlag))
        return NETCFG_TYPE_FAIL;

    //step1: write to ND_MGR
    if(NSM_TYPE_RESULT_OK != NSM_PMGR_SetRaManagedConfigFlag( vid_ifindex, enableFlag))
        return NETCFG_TYPE_FAIL;
    //step2: write to OM
    if(!NETCFG_OM_ND_SetNdManagedConfigFlag( vid_ifindex, enableFlag))
    {
        NSM_PMGR_SetRaManagedConfigFlag( vid_ifindex, pre_enableFlag);
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdManagedConfigFlag(UI32_T vid_ifindex, BOOL_T* enableFlag)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    /*check end*/

    if ( TRUE == NETCFG_OM_ND_GetNdManagedConfigFlag( vid_ifindex, enableFlag))
    {
        if (SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_MANAGED_ADDRESS != *enableFlag)
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}
UI32_T NETCFG_MGR_ND_SetNdOtherConfigFlag(UI32_T vid_ifindex, BOOL_T enableFlag)
{
    BOOL_T      pre_enableFlag;//for rollback

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    //if(pre_msec < low_bound || pre_msec > high_bound)
    //    return NETCFG_TYPE_INVALID_ARG;
    /*check end*/



    if(!NETCFG_OM_ND_GetNdOtherConfigFlag(vid_ifindex,&pre_enableFlag))
        return NETCFG_TYPE_FAIL;

    //step1: write to ND_MGR
    if(NSM_TYPE_RESULT_OK != NSM_PMGR_SetRaOtherConfigFlag( vid_ifindex, enableFlag))
        return NETCFG_TYPE_FAIL;
    //step2: write to OM
    if(!NETCFG_OM_ND_SetNdOtherConfigFlag( vid_ifindex, enableFlag))
    {
        NSM_PMGR_SetRaOtherConfigFlag( vid_ifindex, pre_enableFlag);
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdOtherConfigFlag(UI32_T vid_ifindex, BOOL_T* enableFlag)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    /*check end*/

    if ( TRUE == NETCFG_OM_ND_GetNdOtherConfigFlag( vid_ifindex, enableFlag))
    {
        if (SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_OTHER_CONFIG != *enableFlag)
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

UI32_T NETCFG_MGR_ND_SetNdRaSuppress(UI32_T vid_ifindex, BOOL_T enableSuppress)
{
    BOOL_T enable = !enableSuppress;
    BOOL_T pre_enable;//for rollback

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(!NETCFG_OM_ND_GetNdRaSuppress(vid_ifindex,&pre_enable))
    {
        DBGprintf("Can not access OM");
        return NETCFG_TYPE_FAIL;
    }

    NOTEprintf(" input vid=%ld, enableSuppress=%d, enable=%d", (long)vid_ifindex, enableSuppress, enable);
    //step1: config to NSM
    if(NSM_TYPE_RESULT_OK !=NSM_PMGR_SetRa( vid_ifindex, enable))
        return NETCFG_TYPE_FAIL;
    //step2: write to OM
    if(!NETCFG_OM_ND_SetNdRaSuppress( vid_ifindex, enableSuppress))
    {
        NSM_PMGR_SetRa(vid_ifindex, pre_enable);
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdRaSuppress(UI32_T vid_ifindex, BOOL_T* enableSuppress)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    /*check end*/

    if ( TRUE == NETCFG_OM_ND_GetNdRaSuppress( vid_ifindex, enableSuppress))
    {
        if (SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_SUPPRESS != *enableSuppress)
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

UI32_T NETCFG_MGR_ND_SetNdRaLifetime(UI32_T vid_ifindex, UI32_T seconds)
{
    UI32_T      pre_seconds, ra_max, ra_min;
    UI32_T      ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    NOTEprintf("vid=%d, seconds=%d",(int)vid_ifindex,(int)seconds);

    if(!NETCFG_OM_ND_GetNdRaInterval(vid_ifindex, &ra_max, &ra_min))
        return NETCFG_TYPE_FAIL;

    /* rfc4861: life time can not small than max ra interval
     * 0 means the router is not to be used as a default router
     */
    if (seconds > NETCFG_MGR_ND_RA_LIFETIME_RANGE_MAX /*|| (seconds != 0 && seconds < ra_max)*/)
        return NETCFG_TYPE_INVALID_ARG;

    if(!NETCFG_OM_ND_GetNdRaLifetime(vid_ifindex,&pre_seconds))
        return NETCFG_TYPE_FAIL;

    ret=ND_MGR_SetRaLifetime( vid_ifindex, seconds);
    if(NETCFG_TYPE_OK != ret)
        return ret;

    if(!NETCFG_OM_ND_SetNdRaLifetime( vid_ifindex, seconds))
    {
        NSM_PMGR_SetRaLifetime( vid_ifindex, pre_seconds);
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}

UI32_T NETCFG_MGR_ND_UnsetNdRaLifetime(UI32_T vid_ifindex)
{
    UI32_T      pre_seconds;//for rollback
    UI32_T      ret;

    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /*check end*/

    NOTEprintf("vid=%d",(int)vid_ifindex);
    if(!NETCFG_OM_ND_GetNdRaLifetime(vid_ifindex,&pre_seconds))
        return NETCFG_TYPE_FAIL;

    //step1: write to ND_MGR
    ret=ND_MGR_UnsetRaLifetime( vid_ifindex);
    if(NETCFG_TYPE_OK != ret)
        return ret;

    //step2: write to OM
    if(!NETCFG_OM_ND_UnsetNdRaLifetime( vid_ifindex))
    {
        NSM_PMGR_SetRaLifetime( vid_ifindex, pre_seconds);
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}

SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdRaLifetime(UI32_T vid_ifindex, UI32_T* seconds)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    /*check end*/

    if(TRUE == NETCFG_OM_ND_IsSetNdRaLifetime(vid_ifindex))
    {
        if ( TRUE == NETCFG_OM_ND_GetNdRaLifetime( vid_ifindex,seconds))
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}


UI32_T NETCFG_MGR_ND_SetNdRaInterval(UI32_T vid_ifindex, UI32_T max, UI32_T min)
{
    UI32_T      pre_max, pre_min;
    UI32_T      ra_lifetime;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if (max < NETCFG_MGR_ND_MAX_RA_INTERVAL_RANGE_MIN || max > NETCFG_MGR_ND_MAX_RA_INTERVAL_RANGE_MAX)
        return NETCFG_TYPE_INVALID_ARG;

    if (min < NETCFG_MGR_ND_MIN_RA_INTERVAL_RANGE_MIN || min > (0.75*max))
        return NETCFG_TYPE_INVALID_ARG;

    if(NETCFG_OM_ND_GetNdRaLifetime(vid_ifindex,&ra_lifetime))
    {
        if (ra_lifetime !=0 && ra_lifetime < max)
        {
            DBGprintf("Life < max RA Interval is not a good idea!");
            /* return NETCFG_TYPE_INVALID_ARG; */
        }
    }

    if(!NETCFG_OM_ND_GetNdRaInterval(vid_ifindex, &pre_max, &pre_min))
        return NETCFG_TYPE_FAIL;

    if(NSM_TYPE_RESULT_OK != NSM_PMGR_SetRaInterval(vid_ifindex, max, min))
        return NETCFG_TYPE_FAIL;

    if(!NETCFG_OM_ND_SetNdRaInterval(vid_ifindex, max, min))
    {
        NSM_PMGR_SetRaInterval(vid_ifindex, pre_max, pre_min);
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}

UI32_T NETCFG_MGR_ND_UnsetNdRaInterval(UI32_T vid_ifindex)
{
    UI32_T  pre_max, pre_min;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(!NETCFG_OM_ND_GetNdRaInterval(vid_ifindex, &pre_max, &pre_min))
        return NETCFG_TYPE_FAIL;

    if(NSM_TYPE_RESULT_OK != NSM_PMGR_UnsetRaInterval( vid_ifindex))
        return NETCFG_TYPE_FAIL;

    if(!NETCFG_OM_ND_UnsetNdRaInterval( vid_ifindex))
    {
        NSM_PMGR_SetRaInterval( vid_ifindex, pre_max, pre_min);
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}

SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdRaInterval(UI32_T vid_ifindex, UI32_T *max_p, UI32_T *min_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if(TRUE == NETCFG_OM_ND_IsSetNdRaInterval(vid_ifindex))
    {
        if ( TRUE == NETCFG_OM_ND_GetNdRaInterval(vid_ifindex, max_p, min_p))
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}


/*
 *                  {NETCFG_TYPE_ND_ROUTER_PERFERENCE_HIGH,
 *                      NETCFG_TYPE_ND_ROUTER_PERFERENCE_MEDIUM,
 *                       NETCFG_TYPE_ND_ROUTER_PERFERENCE_LOW}
*/
UI32_T NETCFG_MGR_ND_SetNdRouterPreference(UI32_T vid_ifindex, UI32_T  prefer)
{
    UI32_T pre_prefer;//for rollback
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    //if(pre_msec < low_bound || pre_msec > high_bound)
    //    return NETCFG_TYPE_INVALID_ARG;
    /*check end*/



    if(!NETCFG_OM_ND_GetNdRouterPreference(vid_ifindex,&pre_prefer))
        return NETCFG_TYPE_FAIL;

    NOTEprintf("vid=%ld, prefer=%ld", (long)vid_ifindex, (long)prefer);
    //step1: write to ND_MGR

    if(NETCFG_TYPE_OK != ND_MGR_SetRaRouterPreference( vid_ifindex, prefer))
        return NETCFG_TYPE_FAIL;

    //step2: write to OM
    if(!NETCFG_OM_ND_SetNdRouterPreference( vid_ifindex, prefer))
    {
        NSM_PMGR_SetRaRouterPreference( vid_ifindex, pre_prefer);
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}

SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdRouterPreference(UI32_T vid_ifindex, UI32_T*  prefer)

//SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ND_GetRunningNdRouterPreference(UI32_T vid_ifindex, UI8_T*  prefer)
{
    /*check start*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    /*check end*/

    if ( TRUE == NETCFG_OM_ND_GetNdRouterPreference( vid_ifindex, prefer))
    {
        if (SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_DEFAULT_ROUTER_PREFERENCE != *prefer)
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

#endif /* #if (SYS_CPNT_IPV6_ROUTING == TRUE) */
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

////////////////////
static L_INET_AddrIp_T NETCFG_MGR_ComposeInetAddr(UI16_T type,UI8_T* addrp,UI16_T prefixLen)
{
    L_INET_AddrIp_T addr;

    memset(&addr, 0, sizeof(addr));
    addr.type = type;
    if(L_INET_ADDR_TYPE_IPV4 == type || L_INET_ADDR_TYPE_IPV4Z == type)
        addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    else if(L_INET_ADDR_TYPE_IPV6 == type || L_INET_ADDR_TYPE_IPV6Z ==type)
        addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    else{printf ("Wooop! something wring!\r\n");}
    memcpy(addr.addr,addrp,addr.addrlen);
    addr.preflen = prefixLen;
    return addr;
}
/*
   compare the prefix of 2 addresses,
   >0 if input 1>input2
   =0 i input1 = input2
   <0 if input1 < input2
*/
static int NETCFG_MGR_prefix_compare(UI8_T* input1,UI8_T* input2, int prefix_len)
{
     int byte_len = prefix_len/8;
     int shift;
     int i;
     for(i=0;i<byte_len;i++)
     {
         if(input1[i]!=input2[i])
             return input1[i]-input2[i];
     }
     if(prefix_len%8>0)
     {
       shift = 8-(prefix_len%8);
       return (input1[byte_len]>>shift) - (input2[byte_len]>>shift);
     }
     else
          return 0;

};

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
static BOOL_T NETCFG_MGR_ND_RAGUARD_LocalDoIngressCheck(
    UI32_T  ing_lport);

static BOOL_T NETCFG_MGR_ND_RAGUARD_LocalGetEgrPortsForRelay(
    UI32_T  ing_lport,
    UI16_T  ing_vid,
    UI8_T   *dst_mac_p,
    UI8_T   egr_pbmp_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

static void NETCFG_MGR_ND_RAGUARD_LocalSendPacket(
    UI16_T  ing_vid,
    UI16_T  ing_cos,
    UI8_T   *src_mac_p,
    UI8_T   *dst_mac_p,
    L_MM_Mref_Handle_T  *mref_p,
    UI32_T  egr_pdu_len,
    UI8_T   *egr_pbmp_p);
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */

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
    UI32_T  member_ifindex)
{
    /* 1. copy the 1st member's property to trunk
     */
    NETCFG_OM_ND_RAGUARD_CopyPortCfgTo(member_ifindex, trunk_ifindex);
}

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
    UI32_T  member_ifindex)
{
    BOOL_T  is_old_any_enabled, is_new_any_enabled;

    is_old_any_enabled = NETCFG_OM_ND_RAGUARD_IsAnyPortEnabled();

    /* 1. apply the trunk's property to member
     */
    NETCFG_OM_ND_RAGUARD_CopyPortCfgTo(trunk_ifindex, member_ifindex);

    is_new_any_enabled = NETCFG_OM_ND_RAGUARD_IsAnyPortEnabled();

    if (is_old_any_enabled != is_new_any_enabled)
    {
#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
        /* if more than one port is enabled, need to trap ra packet to cpu
         */
        if (TRUE == is_new_any_enabled)
        {
            /* enable trap packet
             */
            SWCTRL_PMGR_SetRaAndRrPacketTrap(TRUE);
        }
        else
        {
            /* disable trap packet
             */
            SWCTRL_PMGR_SetRaAndRrPacketTrap(FALSE);
        }
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */
    }

#if (SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE == TRUE)
    {
        BOOL_T  is_trk_enabled;

        is_trk_enabled =NETCFG_OM_ND_RAGUARD_IsEnabled(
                trunk_ifindex, NETCFG_TYPE_RG_PKT_MAX);

        SWCTRL_PMGR_SetPortRaAndRrPacketDrop(member_ifindex, is_trk_enabled);
    }
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE == TRUE) */
}

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
    UI32_T  member_ifindex)
{
    /* 1. clear trunk's property
     */
    NETCFG_OM_ND_RAGUARD_SetPortStatus(
        trunk_ifindex, SYS_DFLT_IPV6_RA_GUARD_PORT_STATUS);
}

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
    UI32_T  member_ifindex)
{
    /* 1. do nothing...
     */
}

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
    BOOL_T  *is_enable_p)
{
    BOOL_T  ret = FALSE;

    if (TRUE == SWCTRL_POM_LogicalPortExisting(lport))
    {
        ret = NETCFG_OM_ND_RAGUARD_GetPortStatus(lport, is_enable_p);
    }

    return ret;
}

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
    BOOL_T  *is_enable_p)
{
    UI32_T  nxt_lport;
    BOOL_T  ret = FALSE;

    nxt_lport = *lport_p;
    while (nxt_lport < SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        nxt_lport++;

        ret =    SWCTRL_POM_LogicalPortExisting(nxt_lport)
              && NETCFG_OM_ND_RAGUARD_GetPortStatus(nxt_lport, is_enable_p);

        if (TRUE == ret)
        {
            *lport_p = nxt_lport;
            break;
        }
    }

    return ret;
}

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
    BOOL_T  *is_enable_p)
{
    SYS_TYPE_Get_Running_Cfg_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (TRUE == NETCFG_OM_ND_RAGUARD_GetPortStatus(lport, is_enable_p))
    {
        if (*is_enable_p == SYS_DFLT_IPV6_RA_GUARD_PORT_STATUS)
        {
            ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }
        else
        {
            ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }

    return ret;
}

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
    BOOL_T  is_enable)
{
    UI32_T                      unit, port, trunk_id,
                                byte_cnt, bit_cnt, lport_idx;
    SWCTRL_Lport_Type_T         lport_type;
    TRK_MGR_TrunkEntry_T        trunk_entry;
    BOOL_T                      ret = FALSE, is_trunk = FALSE;
    BOOL_T                      is_old_any_enabled, is_new_any_enabled;

    lport_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    switch (lport_type)
    {
    case SWCTRL_LPORT_TRUNK_PORT:
        trunk_entry.trunk_index = trunk_id;
        /* trunk can be configured when it has at least one member port
         */
        if (  (0 < TRK_PMGR_GetTrunkMemberCounts(trunk_id))
            &&(TRUE == TRK_PMGR_GetTrunkEntry(&trunk_entry))
           )
        {
            is_trunk = TRUE;
            ret      = TRUE;
        }
        break;
    case SWCTRL_LPORT_NORMAL_PORT:
        ret = TRUE;
        break;
    default:
        break;
    }

    if (TRUE == ret)
    {
        if (NETCFG_OM_ND_RAGUARD_IsEnabled(
                lport, NETCFG_TYPE_RG_PKT_MAX) == is_enable)
        {
            /* status is not changed, return TRUE;
             */
            return ret;
        }

#if (SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE == TRUE)
        if (FALSE == SWCTRL_PMGR_SetPortRaAndRrPacketDrop(lport, is_enable))
            return FALSE;
#endif

        is_old_any_enabled = NETCFG_OM_ND_RAGUARD_IsAnyPortEnabled();
        ret = NETCFG_OM_ND_RAGUARD_SetPortStatus(lport, is_enable);
    }

    /* apply the setting to all member port if it's a trunk
     */
    if ((TRUE == ret) && (TRUE == is_trunk))
    {
        for (byte_cnt = 0; byte_cnt < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST; ++byte_cnt)
        {
            if ( 0 != trunk_entry.trunk_ports[byte_cnt])
            {
                for (bit_cnt = 0; bit_cnt < 8; ++bit_cnt)
                {
                    if ((trunk_entry.trunk_ports[byte_cnt] & (0x01 << (7 - bit_cnt))) != 0)
                    {
                        lport_idx = byte_cnt * 8 + bit_cnt + 1;
                        NETCFG_OM_ND_RAGUARD_SetPortStatus(lport_idx, is_enable);
                    }
                }
            }
        }
    }

    if (TRUE == ret)
    {
        /* if more than one port is enabled, need to trap ra packet to cpu
         */
        is_new_any_enabled = NETCFG_OM_ND_RAGUARD_IsAnyPortEnabled();

        if (is_old_any_enabled != is_new_any_enabled)
        {
#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
            if (TRUE == is_new_any_enabled)
            {
                /* enable trap packet
                 */
                SWCTRL_PMGR_SetRaAndRrPacketTrap(TRUE);
            }
            else
            {
                /* disable trap packet
                 */
                SWCTRL_PMGR_SetRaAndRrPacketTrap(FALSE);
            }
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */
        }
    }

    return ret;
}

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
    BOOL_T  use_default)
{
    /* 1. for global setting,   need to re-config the driver setting
     *
     * 2. for per port setting, cli will re-provision the setting
     *                          when inserting back
     */
#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
    if (TRUE == NETCFG_OM_ND_RAGUARD_IsAnyPortEnabled())
    {
        SWCTRL_PMGR_SetRaAndRrPacketTrap(TRUE);
    }
#endif

}

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
    UI32_T  number_of_port)
{
    UI32_T              ifindex;

    /* 1. for global setting,   do nothing
     *                          (bcz cli will not record it)
     * 2. for per port setting, clear the om
     */
    for (ifindex = starting_port_ifindex;
         ifindex<= starting_port_ifindex+number_of_port-1;
         ifindex++)
    {
        NETCFG_OM_ND_RAGUARD_SetPortStatus(ifindex, FALSE);
    }

#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
    if (FALSE == NETCFG_OM_ND_RAGUARD_IsAnyPortEnabled())
    {
        SWCTRL_PMGR_SetRaAndRrPacketTrap(FALSE);
    }
#endif
}

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
    UI32_T              src_lport)
{
    UI32_T  drop_cnt =1, ing_pdu_len=0;
    UI8_T   *ing_pdu_p;
    UI8_T   egr_lports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    BOOL_T  rc;

    if (NULL == mref_handle_p)
    {
        return;
    }

    if ((NULL != dst_mac_p) && (NULL != src_mac_p))
    {
        ing_pdu_p = L_MM_Mref_GetPdu(mref_handle_p, &ing_pdu_len);

        if (TRUE == NETCFG_MGR_ND_RAGUARD_LocalDoIngressCheck(
                            src_lport))
        {
            rc = NETCFG_MGR_ND_RAGUARD_LocalGetEgrPortsForRelay(
                            src_lport,
                            ing_vid,
                            dst_mac_p,
                            egr_lports);

            if (TRUE == rc)
            {
                NETCFG_MGR_ND_RAGUARD_LocalSendPacket(
                            ing_vid,
                            ing_cos,
                            src_mac_p,
                            dst_mac_p,
                            mref_handle_p,
                            ing_pdu_len,
                            egr_lports);
                drop_cnt = 0;
            }
        }
    }

    if (1 == drop_cnt)
    {
        L_MM_Mref_Release(&mref_handle_p);
    }

    DBGprintf("\r\nport/pkt_len/ing_vid/ing_cos/pkt_type/rcv/drop-%ld/%ld/%d/%d/%d/%d/%ld\r\n",
            (long)src_lport, (long)pkt_length, ing_vid, ing_cos, pkt_type, 1, (long)drop_cnt);

    /*   counter will be updated in IML_MGR_PacketClassification.
     *
     *   NETCFG_OM_ND_RAGUARD_IncStatistics(
     *             src_lport, pkt_type, 1, drop_cnt);
     */
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_RAGUARD_LocalDoIngressCheck
 * ------------------------------------------------------------------------
 * PURPOSE: To check if packet received on ing_lport should be processed.
 * INPUT  : ing_lport - which lport to check
 * OUTPUT : None
 * RETURN : TRUE  - need further processing
 *          FALSE - drop
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
static BOOL_T NETCFG_MGR_ND_RAGUARD_LocalDoIngressCheck(
    UI32_T  ing_lport)
{
    BOOL_T  ret = FALSE;

    if (FALSE == NETCFG_OM_ND_RAGUARD_IsEnabled(
                    ing_lport, NETCFG_TYPE_RG_PKT_MAX))
    {
        ret = TRUE;
    }

    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_RAGUARD_LocalGetEgrPortsForRelay
 * ------------------------------------------------------------------------
 * PURPOSE: To get egress port bitmap for relay.
 * INPUT  : ing_lport    - ingress lport
 *          ing_vid      - ingress vid
 *          dst_mac_p    - pointer to dst mac
 * OUTPUT : egr_pbmp_ar  - pointer to egress port bitmap
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
static BOOL_T NETCFG_MGR_ND_RAGUARD_LocalGetEgrPortsForRelay(
    UI32_T  ing_lport,
    UI16_T  ing_vid,
    UI8_T   *dst_mac_p,
    UI8_T   egr_pbmp_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    UI32_T                  egr_port_num = 0, lport_byte;
    AMTR_TYPE_AddrEntry_T   addr_entry = {0};
    UI8_T                   lport_byte_mask;
    BOOL_T                  is_broadcast = TRUE,
                            ret = FALSE;

    memset(egr_pbmp_ar, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    /* check if unicast
     */
    if (0 == (dst_mac_p[0] & 0x1))
    {
        memcpy(addr_entry.mac, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN);
        addr_entry.vid = ing_vid;

        if (TRUE == AMTR_PMGR_GetExactAddrEntry(&addr_entry))
        {
            if (addr_entry.ifindex != ing_lport)
            {
                lport_byte_mask = (1 << (7 - ((addr_entry.ifindex - 1) & 7)));
                lport_byte      = (addr_entry.ifindex - 1) >> 3;
                egr_pbmp_ar[lport_byte] |= lport_byte_mask;
                egr_port_num = 1;
            }

            is_broadcast = FALSE;
        }
    }

    if (TRUE == is_broadcast)
    {
        /* broadcast
         */
        memset(egr_pbmp_ar, 0xff, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

        /* remove the ingress port
         */
        lport_byte_mask = (1 << (7 - ((ing_lport - 1) & 7)));
        lport_byte      = (ing_lport - 1) >> 3;
        egr_port_num = 1; /* set this to 1 to return TRUE */
        egr_pbmp_ar[lport_byte] &= ~lport_byte_mask;
    }

    if (egr_port_num > 0)
    {
        ret = TRUE;
    }

    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ND_RAGUARD_LocalSendPacket
 * ------------------------------------------------------------------------
 * PURPOSE: To send packet via specified egress port bitmap.
 * INPUT  : ing_vid       - ingress vid
 *          ing_cos       - ingress cos
 *          src_mac_p     - pointer to src mac
 *          dst_mac_p     - pointer to dst mac
 *          mref_p        - pointer to egress mref
 *          egr_pdu_len   - length of egress pdu
 *          egr_pbmp_p    - pointer to egress port bitmap
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
static void NETCFG_MGR_ND_RAGUARD_LocalSendPacket(
    UI16_T  ing_vid,
    UI16_T  ing_cos,
    UI8_T   *src_mac_p,
    UI8_T   *dst_mac_p,
    L_MM_Mref_Handle_T  *mref_p,
    UI32_T  egr_pdu_len,
    UI8_T   *egr_pbmp_p)
{
    if ((NULL != mref_p) && (NULL != egr_pbmp_p))
    {
        /* send packet */
        L2MUX_PMGR_SendMultiPacketByVlan(
              mref_p,                           /* L_MREF        */
              dst_mac_p,                        /* dst mac       */
              src_mac_p,                        /* src mac       */
              0x86DD,                           /* packet type   */
              (ing_vid | ing_cos << 13),        /* tag_info      */
              egr_pdu_len,                      /* packet length */
              egr_pbmp_p,                       /* lport         */
              ing_cos);
    }
}
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */


