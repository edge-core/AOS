/*
 *   File Name: ipal_arp_mgr.c
 *   Purpose:   TCP/IP shim layer(ipal) ARP management implementation
 *   Note:
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify		   Date      Reason
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
*/

/*
 * INCLUDE FILE DECLARATIONS
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "sys_type.h"
#include "sys_adpt.h"

#include "l_threadgrp.h"
#include "backdoor_mgr.h"

#include "l_inet.h"
#include "l_prefix.h"
#include "ipal_types.h"
#include "ipal_icmp.h"
#include "ipal_neigh.h"
#include "ipal_route.h"
#include "ipal_if.h"
#include "ipal_rt_netlink.h"
#include "ipal_debug.h"
#include "netcfg_proc_comm.h"

#include <linux/if.h>

/*
 * NAMING CONST DECLARATIONS
*/



/*
 * MACRO FUNCTION DECLARATIONS
*/

#define IPAL_BACKDOOR_Printf(fmt_p, ...) BACKDOOR_MGR_Printf((fmt_p), ##__VA_ARGS__)
#define IPAL_BACKDOOR_GetLine(buf, size) ({ BACKDOOR_MGR_RequestKeyIn((buf), (size)); strlen(buf); })

/*
 * DATA TYPE DECLARATIONS
*/


/*
 * STATIC VARIABLE DECLARATIONS
 */

static L_THREADGRP_Handle_T tg_handle;
static UI32_T backdoor_member_id;


/*
 * LOCAL SUBPROGRAM DECLARATIONS
*/
static void IPAL_BACKDOOR_Main(void);

static void IPAL_BACKDOOR_MainMenu(void);
#if 0
static void IPAL_BACKDOOR_IPConfig(void);
#endif
static void IPAL_BACKDOOR_ARPConfig(void);
#if (SYS_CPNT_IPV6 == TRUE)
static void IPAL_BACKDOOR_Ipv6NeighConfig(void);
#endif
static void IPAL_BACKDOOR_UCRouteConfig(void);
static void IPAL_BACKDOOR_InterfaceConfig(void);
static void IPAL_BACKDOOR_IcmpConfig(void);
static void IPAL_BACKDOOR_RouteConfig(void);

#define MAC_ADDRESS_SIZE                6
#define MAC_ADDRESS_MIN_STR_LEN         sizeof ("FFFFFFFFFFFF")
#define MAC_ADDRESS_MAX_STR_LEN         sizeof ("FF-FF-FF-FF-FF-FF")
#define MAC_ADDRESS_SEPERATOR_STR       "-"     /* ":"  */
#define MAC_ADDRESS_SEPERATOR_CHAR      '-'     /* ':'  */

typedef struct MAC_ADDRESS_S            MAC_ADDRESS_T;

struct MAC_ADDRESS_S
{
    UI8_T   addr[MAC_ADDRESS_SIZE];
} __ATTRIBUTE_PACKED__;


/*
 * FUNCTION: AMS_MACADDR_Aton
 *
 * PURPOSE:
 *  convert the ethernet mac string to the binary form.
 *
 * INPUT:
 *  mac_addr_string         -   an  ethernet mac address string
 *
 * OUTPUT:
 *  mac_addr  -  mac address binary form.
 *
 * RETURN:
 *      TRUE       Succeed
 *      FALSE      Failed
 * NOTES:
 *    None.
 */
BOOL_T
AMS_MACADDR_Aton(const char *mac_str, MAC_ADDRESS_T *mac)
{
    size_t          len;
    size_t          index;
    MAC_ADDRESS_T   mac_tmp;
    char            dstr[3] = {'X', 'X', '\0' };

    if ((mac_str == NULL) || (*mac_str == '\0'))
        return FALSE;

    while (isspace(*mac_str))
        mac_str++;

    len = strlen(mac_str);

    if (len < (MAC_ADDRESS_MIN_STR_LEN - 1))
        return FALSE;

    for (index = 0; index < MAC_ADDRESS_SIZE; index++)
    {
        /*
         * Supported MAC address string styles (seperator locates at even position):
         *
         * XX-XX-XX-XX-XX-XX    XX:XX:XX:XX:XX:XX
         * XXXX-XXXX-XXXX       XXXX:XXXX:XXXX
         * XXXXXX-XXXXXX        XXXXXX:XXXXXX
         * XXXXXXXXXXXX
         *
         */

        if ((*mac_str == '\0') || !(isxdigit(mac_str[0]) && isxdigit(mac_str[1])))
            return FALSE;

        dstr[0]             = *mac_str++;
        dstr[1]             = *mac_str++;
        mac_tmp.addr[index] = (UI8_T) strtoul(dstr, NULL, 16);

        if (*mac_str && ((*mac_str == '-') || (*mac_str == ':')))
            mac_str++;
    }

    if (mac)    /* NULL mac means caller just check the validity of address string  */
        memcpy(mac, &mac_tmp, sizeof (MAC_ADDRESS_T));

    return TRUE;
}


void IPAL_BACKDOOR_Create_InterCSC_Relation()
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("ipal", SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY, IPAL_BACKDOOR_Main);
}


static void IPAL_BACKDOOR_Main(void)
{
    tg_handle = NETCFG_PROC_COMM_GetNetcfgTGHandle();
    /* Join thread group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_NETCFG_GROUP_MGR_THREAD_PRIORITY, &backdoor_member_id) == FALSE)
    {
        IPAL_BACKDOOR_Printf("\r\n%s: L_THREADGRP_Join fail.", __FUNCTION__);
        return;
    }
    IPAL_Rt_Netlink_Init ();
    IPAL_BACKDOOR_MainMenu();

    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
}


static void IPAL_BACKDOOR_MainMenu(void)
{
    #define MAXLINE 255
    char line_buffer[MAXLINE];
    int  select_value = 0;

    while (1)
    {
        IPAL_BACKDOOR_Printf("\r\n------------- IPAL Backdoor Main DB Menu ------------");
        IPAL_BACKDOOR_Printf("\r\n\t 1 : Interface Configuration");
        IPAL_BACKDOOR_Printf("\r\n\t 2 : Ipv4 Arp Configuration");
#if (SYS_CPNT_IPV6 == TRUE)
        IPAL_BACKDOOR_Printf("\r\n\t 3 : Ipv6 Neighbor Configuration");
#endif
        IPAL_BACKDOOR_Printf("\r\n\t 4 : UCRoute Configuration");
        IPAL_BACKDOOR_Printf("\r\n\t 5 : Interface Configuration");
        IPAL_BACKDOOR_Printf("\r\n\t 6 : ICMP Configuration");
        IPAL_BACKDOOR_Printf("\r\n\t 7 : Route Configuration");
        IPAL_BACKDOOR_Printf("\r\n\t 0 : Return");
        IPAL_BACKDOOR_Printf("\r\n--------------------------------------------------------\r\n");
        IPAL_BACKDOOR_Printf("\r\nEnter Selection:");

        if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        switch (select_value)
        {
            case 1:
                IPAL_BACKDOOR_InterfaceConfig();
                 break;
            case 2:
                IPAL_BACKDOOR_ARPConfig();
                break;
#if (SYS_CPNT_IPV6 == TRUE)
            case 3:
                IPAL_BACKDOOR_Ipv6NeighConfig();
                break;
#endif
            case 4:
                IPAL_BACKDOOR_UCRouteConfig();
                break;
            case 5:
                IPAL_BACKDOOR_InterfaceConfig();
                break;
            case 6:
                IPAL_BACKDOOR_IcmpConfig();
                break;
            case 7:
                IPAL_BACKDOOR_RouteConfig();
                break;
            case 0:
                IPAL_BACKDOOR_Printf("\r\n");
                return;
            default:
                break;
        }
    }
}
#if 0
static void IPAL_BACKDOOR_IPConfig(void)
{
    #define MAXLINE 255
    UI32_T  select_value = 0;
    char   line_buffer[MAXLINE];
    int ip_forward;

    while (1)
    {
        IPAL_BACKDOOR_Printf("\r\n----------IPAL IP Configuration ---------------");
        IPAL_BACKDOOR_Printf("\r\n\t 1 : Enable IP Forwarding");
        IPAL_BACKDOOR_Printf("\r\n\t 2 : Disable IP Forwarding");
        IPAL_BACKDOOR_Printf("\r\n\t 3 : Show IP Forwarding Status");
        IPAL_BACKDOOR_Printf("\r\n\t 0 : Return");
        IPAL_BACKDOOR_Printf("\r\n--------------------------------------------------------\r\n");
        IPAL_BACKDOOR_Printf("\r\nEnter Selection:");

        if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        switch (select_value)
        {
            case 1:
                if(IPAL_Enable_Ipv4Forwarding () != IPAL_RESULT_OK)
                {
                    IPAL_BACKDOOR_Printf("\r\nEnable IP Forwarding Fail ret=%d\r\n", errno);
                }
                break;
            case 2:
                if(IPAL_Disable_Ipv4Forwarding () != IPAL_RESULT_OK)
                {
                    IPAL_BACKDOOR_Printf("\r\nDisable IP Forwarding Fail ret=%d\r\n", errno);
                }
                break;
            case 3:
                if(IPAL_Get_Ipv4Forwarding (&ip_forward) != IPAL_RESULT_OK)
                {
                    IPAL_BACKDOOR_Printf("\r\nGet IP Forwarding Status Fail ret=%d\r\n", errno);
                }
                IPAL_BACKDOOR_Printf("\r\nIP Forwarding (1. Enable, 0. Disable) is %d \r\n", ip_forward);
                break;
            case 0:
                IPAL_BACKDOOR_Printf("\r\n");
                return;
            default:
                break;
        }
    }

}
#endif

static void IPAL_BACKDOOR_ARPConfig(void)
{
    #define MAXLINE 255
    UI32_T  select_value = 0, set_value = 0;
    UI32_T ifindex;
    UI32_T arp_type;
    char   line_buffer[MAXLINE];
    UI8_T    mac_addr[SYS_ADPT_MAC_ADDR_LEN] = {0};
    MAC_ADDRESS_T mac;
    BOOL_T arp_proxy;
    UI32_T   timeout;
    IPAL_NeighborEntry_T neigh_entry[10];
    UI32_T               neigh_entry_num;
    L_INET_AddrIp_T ipaddr;
    UI32_T          i;

    while (1)
    {
        IPAL_BACKDOOR_Printf("\r\n----------IPAL ARP Configuration ---------------");
        IPAL_BACKDOOR_Printf("\r\n\t 1  : Show IP Stack ARP Table");
        IPAL_BACKDOOR_Printf("\r\n\t 2  : Show Interface ARP Proxy Status");
        IPAL_BACKDOOR_Printf("\r\n\t 3  : Enable Interface ARP Proxy ");
        IPAL_BACKDOOR_Printf("\r\n\t 4  : Disable Interface ARP Proxy");
        IPAL_BACKDOOR_Printf("\r\n\t 5  : Add Arp Entry.");
        IPAL_BACKDOOR_Printf("\r\n\t 6  : Del Arp Entry.");
        IPAL_BACKDOOR_Printf("\r\n\t 7  : Flush All Dynamic ARP Table");
        IPAL_BACKDOOR_Printf("\r\n\t 8  : Set arp aging timeout");
        IPAL_BACKDOOR_Printf("\r\n\t 9  : show arp aging timeout");
        IPAL_BACKDOOR_Printf("\r\n\t 10 : get arp entry");
        IPAL_BACKDOOR_Printf("\r\n\t 11 : get next arp entry");
        IPAL_BACKDOOR_Printf("\r\n\t 12 : send a arp request");
        IPAL_BACKDOOR_Printf("\r\n\t 0 : Return");
        IPAL_BACKDOOR_Printf("\r\n--------------------------------------------------------\r\n");
        IPAL_BACKDOOR_Printf("\r\nEnter Selection:");

        if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        switch (select_value)
        {
            case 1:
                IPAL_NEIGH_ShowIpv4NeighTable();
                break;
            case 2:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface(ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                if(IPAL_NEIGH_GetIpv4ProxyArp(ifindex, &arp_proxy) != IPAL_RESULT_OK )
                    IPAL_BACKDOOR_Printf("\r\nGet Interface Arp Proxy Status Fail ret=%d\r\n", errno);
                else
                IPAL_BACKDOOR_Printf("\r\nInterface Arp Proxy Status(1 Enable, 0 Disable) is =%d\r\n", arp_proxy);
                break;
#if (SYS_CPNT_PROXY_ARP == TRUE)				
            case 3:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                if( IPAL_NEIGH_EnableIpv4ProxyArp(ifindex) != IPAL_RESULT_OK )
                    IPAL_BACKDOOR_Printf("\r\nEnable Interface Arp Proxy Status Fail ret=%d\r\n", errno);
                else
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                break;
            case 4:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                if( IPAL_NEIGH_DisableIpv4ProxyArp(ifindex) != IPAL_RESULT_OK )
                    IPAL_BACKDOOR_Printf("\r\nDisable Interface Arp Proxy Status Fail ret=%d\r\n", errno);
                else
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                break;
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */				
            case 5:
                IPAL_BACKDOOR_Printf("\r\nSelect interface(ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface ipaddr (xxx.xxx.xxx.xxx): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_INET_StringToInaddr(L_INET_FORMAT_IPV4_UNSPEC, line_buffer,
                        (L_INET_Addr_T *)&ipaddr, sizeof(ipaddr));
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface mac (xx-xx-xx-xx-xx-xx) : ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    AMS_MACADDR_Aton(line_buffer, &mac);
                }
                IPAL_BACKDOOR_Printf("\r\nSelect Arp type(4=static,3=dynamic): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    arp_type = atoi(line_buffer);
                }
                if (IPAL_NEIGH_AddNeighbor(ifindex, &ipaddr, 6,  mac.addr, arp_type, TRUE)==IPAL_RESULT_OK)
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 6:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface(ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    set_value = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nEnter interface ipaddr value (xxx.xxx.xxx.xxx): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_INET_StringToInaddr(L_INET_FORMAT_IPV4_UNSPEC, line_buffer,
                        (L_INET_Addr_T *)&ipaddr, sizeof(ipaddr));
                }
                if (IPAL_NEIGH_DeleteNeighbor(ifindex, &ipaddr)==IPAL_RESULT_OK)
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else
                    IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 7:
                if (IPAL_NEIGH_FlushAllDynamicNeighbor(IPAL_NEIGHBOR_ADDR_TYPE_IPV4)==IPAL_RESULT_OK)
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else
                    IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 8:
                IPAL_BACKDOOR_Printf("\r\nEnter arp entry aging timeout value: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    timeout = atoi(line_buffer);
                }
                if(IPAL_NEIGH_SetNeighborAgingTime(timeout) != IPAL_RESULT_OK)
                	IPAL_BACKDOOR_Printf("\r\nSet Interface Arp Aging timeout  Fail ret=%d, %s\r\n", errno, strerror(errno));
                else
                    IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 9:
                timeout = 0;
                if(IPAL_NEIGH_GetNeighborAgingTime(&timeout) != IPAL_RESULT_OK)
                	IPAL_BACKDOOR_Printf("\r\nGet Arp aging timeout  Fail ret=%d, %s\r\n", errno, strerror(errno));
                else
                    IPAL_BACKDOOR_Printf("\r\nArp entry aging timeout is %d \r\n", timeout);
                break;
            case 10:
	            IPAL_BACKDOOR_Printf("\r\nSelect interface(ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface ipaddr (xxx.xxx.xxx.xxx): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_INET_StringToInaddr(L_INET_FORMAT_IPV4_UNSPEC, line_buffer,
                        (L_INET_Addr_T *)&ipaddr, sizeof(ipaddr));
                }
                if(IPAL_NEIGH_GetNeighbor(ifindex, &ipaddr, &neigh_entry[0]) == IPAL_RESULT_OK)
                {
                	IPAL_BACKDOOR_Printf("\r\n%02X-%02X-%02X-%02X-%02X-%02X\n",
	                 neigh_entry[0].phy_address[0], neigh_entry[0].phy_address[1],
	                 neigh_entry[0].phy_address[2], neigh_entry[0].phy_address[3],
	                 neigh_entry[0].phy_address[4], neigh_entry[0].phy_address[5]);
                }
                else
                    IPAL_BACKDOOR_Printf("\r\n get arp entry fail\n ");
                break;

	        case 11:
	            IPAL_BACKDOOR_Printf("\r\nSelect interface(ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface ipaddr (xxx.xxx.xxx.xxx): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    memset(&ipaddr, 0x0, sizeof(L_INET_AddrIp_T));
                    L_INET_StringToInaddr(L_INET_FORMAT_IPV4_UNSPEC, line_buffer,
                        (L_INET_Addr_T *)&ipaddr, sizeof(ipaddr));
                }
	            IPAL_BACKDOOR_Printf("\r\nEnter get number (max 10): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    neigh_entry_num = atoi(line_buffer);
                    if (neigh_entry_num>10)
                        neigh_entry_num = 10;
                }
                if(IPAL_NEIGH_GetNextNNeighbor(ifindex, &ipaddr, &neigh_entry_num, &neigh_entry[0]) == IPAL_RESULT_OK)
                {
                    for(i=0; i<neigh_entry_num; i++)
                    {
                	    IPAL_BACKDOOR_Printf("\r\nifindex: %d ipaddr: %d.%d.%d.%d mac:%02X-%02X-%02X-%02X-%02X-%02X\n",
                	        neigh_entry[i].ifindex, neigh_entry[i].ip_addr.addr[0],neigh_entry[i].ip_addr.addr[1], neigh_entry[i].ip_addr.addr[2],
                	        neigh_entry[i].ip_addr.addr[3],
	                        neigh_entry[i].phy_address[0], neigh_entry[i].phy_address[1],
	                        neigh_entry[i].phy_address[2], neigh_entry[i].phy_address[3],
	                        neigh_entry[i].phy_address[4], neigh_entry[i].phy_address[5]);
                    }
                }
                else
                    IPAL_BACKDOOR_Printf("\r\n get next arp entry fail\n ");
                break;
            case 12:
	            IPAL_BACKDOOR_Printf("\r\nSelect interface(ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface ipaddr (xxx.xxx.xxx.xxx): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_INET_StringToInaddr(L_INET_FORMAT_IPV4_UNSPEC, line_buffer,
                        (L_INET_Addr_T *)&ipaddr, sizeof(ipaddr));
                }
                /*IPAL_BACKDOOR_Printf("\r\nenter type(0=false, 1= true): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    type = atoi(line_buffer);
                }*/
                if(IPAL_NEIGH_SendNeighborRequest(ifindex, &ipaddr) == IPAL_RESULT_OK)
                {
                	IPAL_BACKDOOR_Printf("ifindex: %d ipaddr: %d.%d.%d.%d send OK\n",
                	 ifindex, ipaddr.addr[0],ipaddr.addr[1],ipaddr.addr[2],ipaddr.addr[3]);
                }
                else
                    IPAL_BACKDOOR_Printf("\r\n arp request send fail\n ");
                break;
            case 0:
                IPAL_BACKDOOR_Printf("\r\n");
                return;
            default:
                break;
        }
    }
}

#if (SYS_CPNT_IPV6 == TRUE)
static void IPAL_BACKDOOR_Ipv6NeighConfig(void)
{
    #define MAXLINE 255
    UI32_T  select_value = 0, set_value = 0;
    UI32_T ifindex;
    UI32_T arp_type;
    char   line_buffer[MAXLINE];
    UI8_T    mac_addr[SYS_ADPT_MAC_ADDR_LEN] = {0};
    MAC_ADDRESS_T mac;
    BOOL_T proxy_ndp;
    UI32_T   timeout;
    IPAL_NeighborEntry_T neigh_entry[10];
    UI32_T               neigh_entry_num;
    L_INET_AddrIp_T ipaddr;
    UI32_T          i;

    while (1)
    {
        IPAL_BACKDOOR_Printf("\r\n----------IPAL IPv6 Configuration ---------------");
        IPAL_BACKDOOR_Printf("\r\n\t 1  : Show IP Stack IPv6 neigh Table");
        IPAL_BACKDOOR_Printf("\r\n\t 2  : Show Interface Proxy NDP Status");
        IPAL_BACKDOOR_Printf("\r\n\t 3  : Enable Interface Proxy NDP");
        IPAL_BACKDOOR_Printf("\r\n\t 4  : Disable Interface Proxy NDP");
        IPAL_BACKDOOR_Printf("\r\n\t 5  : Add IPv6 neigh Entry.");
        IPAL_BACKDOOR_Printf("\r\n\t 6  : Del IPv6 neigh Entry.");
        IPAL_BACKDOOR_Printf("\r\n\t 7  : Flush All Dynamic IPv6 neigh Table");
        IPAL_BACKDOOR_Printf("\r\n\t 8  : Set IPv6 neigh table aging timeout");
        IPAL_BACKDOOR_Printf("\r\n\t 9  : show IPv6 neigh table aging timeout");
        IPAL_BACKDOOR_Printf("\r\n\t 10 : get IPv6 neigh entry");
        IPAL_BACKDOOR_Printf("\r\n\t 11 : get next IPv6 neigh entry");
        IPAL_BACKDOOR_Printf("\r\n\t 12 : send a IPv6 neigh solicitation");
        IPAL_BACKDOOR_Printf("\r\n\t 0 : Return");
        IPAL_BACKDOOR_Printf("\r\n--------------------------------------------------------\r\n");
        IPAL_BACKDOOR_Printf("\r\nEnter Selection:");

        if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        switch (select_value)
        {
            case 1:
                 IPAL_NEIGH_ShowIpv6NeighTable();
                break;
            case 2:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface(ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                if(IPAL_NEIGH_GetIpv6ProxyNdp(ifindex, &proxy_ndp) != IPAL_RESULT_OK )
                {
                    IPAL_BACKDOOR_Printf("\r\nGet Interface Proxy Ndp status Fail ret=%d\r\n", errno);
                }
                IPAL_BACKDOOR_Printf("\r\nInterface Proxy Ndp Status(1 Enable, 0 Disable) is =%d\r\n", proxy_ndp);
                break;
            case 3:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    set_value = atoi(line_buffer);
                }

                if( IPAL_NEIGH_EnableIpv6ProxyNdp(set_value) != IPAL_RESULT_OK )
                {
                    IPAL_BACKDOOR_Printf("\r\nEnable Interface Proxy Ndp Fail ret=%d\r\n", errno);
                }
                else
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                break;
            case 4:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    set_value = atoi(line_buffer);
                }
                if( IPAL_NEIGH_DisableIpv6ProxyNdp(set_value) != IPAL_RESULT_OK )
                {
                    IPAL_BACKDOOR_Printf("\r\nDisable Interface Proxy Ndp Fail ret=%d\r\n", errno);
                }
                else
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                break;
            case 5:
                IPAL_BACKDOOR_Printf("\r\nSelect interface(ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface ipv6 addr (xxxx::xxxx:..:xxxx): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC, line_buffer,
                        (L_INET_Addr_T *)&ipaddr, sizeof(ipaddr));
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface mac (xx-xx-xx-xx-xx-xx) : ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    AMS_MACADDR_Aton(line_buffer, &mac);
                }
                IPAL_BACKDOOR_Printf("\r\nSelect Neighbor type(0=static,1=dynamic): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    arp_type = atoi(line_buffer);
                }
                if (IPAL_NEIGH_AddNeighbor(ifindex, &ipaddr, 6,  mac.addr, arp_type, TRUE)==IPAL_RESULT_OK)
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 6:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface(ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nEnter interface ipv6 addr value (xxxx::xxxx:..:xxxx): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC, line_buffer,
                        (L_INET_Addr_T *)&ipaddr, sizeof(ipaddr));
                }
                if (IPAL_NEIGH_DeleteNeighbor(ifindex, &ipaddr)==IPAL_RESULT_OK)
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 7:
                if (IPAL_NEIGH_FlushAllDynamicNeighbor(IPAL_NEIGHBOR_ADDR_TYPE_IPV6)==IPAL_RESULT_OK)
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else
                    IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 8:
                IPAL_BACKDOOR_Printf("\r\nEnter neighbor entry aging timeout value: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    timeout = atoi(line_buffer);
                }
                if(IPAL_NEIGH_SetNeighborAgingTime(timeout) != IPAL_RESULT_OK)
                {
                	IPAL_BACKDOOR_Printf("\r\nSet interface neighbor aging timeout  Fail ret=%d, %s\r\n", errno, strerror(errno));
                }
                else
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                break;
            case 9:
                timeout = 0;
                if(IPAL_NEIGH_GetNeighborAgingTime(&timeout) != IPAL_RESULT_OK)
                {
                	IPAL_BACKDOOR_Printf("\r\nGet neighbor aging timeout  Fail ret=%d, %s\r\n", errno, strerror(errno));
                }
                else
                    IPAL_BACKDOOR_Printf("\r\nNeighbor entry aging timeout is %d \r\n", timeout);
                break;
            case 10:
	            IPAL_BACKDOOR_Printf("\r\nSelect interface(ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface ipv6 addr (xxxx::xxxx:..:xxxx): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC, line_buffer,
                        (L_INET_Addr_T *)&ipaddr, sizeof(ipaddr));
                }
                if(IPAL_NEIGH_GetNeighbor(ifindex, &ipaddr, &neigh_entry[0]) == IPAL_RESULT_OK)
                {
                	IPAL_BACKDOOR_Printf("\r\n%02X-%02X-%02X-%02X-%02X-%02X\n",
	                 neigh_entry[0].phy_address[0], neigh_entry[0].phy_address[1],
	                 neigh_entry[0].phy_address[2], neigh_entry[0].phy_address[3],
	                 neigh_entry[0].phy_address[4], neigh_entry[0].phy_address[5]);
                }
                else
                    IPAL_BACKDOOR_Printf("\r\n get neighbor entry fail\n ");
                break;

	        case 11:
	            IPAL_BACKDOOR_Printf("\r\nSelect interface(ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface ipv6 addr (xxxx::xxxx:..:xxxx): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    memset(&ipaddr, 0x0, sizeof(L_INET_AddrIp_T));
                    L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC, line_buffer,
                        (L_INET_Addr_T *)&ipaddr, sizeof(ipaddr));
                }
	            IPAL_BACKDOOR_Printf("\r\nEnter get number (max 10): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    neigh_entry_num = atoi(line_buffer);
                    if (neigh_entry_num>10)
                        neigh_entry_num = 10;
                }
                if(IPAL_NEIGH_GetNextNNeighbor(ifindex, &ipaddr, &neigh_entry_num, neigh_entry) == IPAL_RESULT_OK)
                {
                    for(i=0; i<neigh_entry_num; i++)
                    {
                        if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&(neigh_entry[i].ip_addr),
                                                                           line_buffer,
                                                                           sizeof(line_buffer)))
                        {
                            IPAL_BACKDOOR_Printf("\r\nsuccess: %s\n", line_buffer);
                        }
                        else IPAL_BACKDOOR_Printf("\r\nsuccess but address error\n");
                	    IPAL_BACKDOOR_Printf("\r\nifindex: %d ipaddr: %s mac:%02X-%02X-%02X-%02X-%02X-%02X\n",
                	        neigh_entry[i].ifindex, line_buffer,
	                        neigh_entry[i].phy_address[0], neigh_entry[i].phy_address[1],
	                        neigh_entry[i].phy_address[2], neigh_entry[i].phy_address[3],
	                        neigh_entry[i].phy_address[4], neigh_entry[i].phy_address[5]);
                    }
                }
                else
                    IPAL_BACKDOOR_Printf("\r\n get next neighbor entry fail\n ");
                break;
            case 12:
	            IPAL_BACKDOOR_Printf("\r\nSelect interface(ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface ipv6 addr (xxxx::xxxx:..:xxxx): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC, line_buffer,
                        (L_INET_Addr_T *)&ipaddr, sizeof(ipaddr));
                }
                /*IPAL_BACKDOOR_Printf("\r\nenter type(0=false, 1= true): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    type = atoi(line_buffer);
                }*/
                if(IPAL_NEIGH_SendNeighborRequest(ifindex, &ipaddr) == IPAL_RESULT_OK)
                {
                    if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&ipaddr,
                                                                       line_buffer,
                                                                       sizeof(line_buffer)))
                    {
                        IPAL_BACKDOOR_Printf("\r\nsuccess: ifindex: %d ipaddr: %s send OK\n", ifindex, line_buffer);
                    }
                    else IPAL_BACKDOOR_Printf("\r\nsuccess but address error\n");
                }
                else
                    IPAL_BACKDOOR_Printf("\r\n neighbor solicitation send fail\n ");
                break;
            case 0:
                IPAL_BACKDOOR_Printf("\r\n");
                return;
            default:
                break;
        }
    }
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

static void IPAL_BACKDOOR_UCRouteConfig(void)
{
    #define MAXLINE 255
    UI32_T  select_value = 0, set_value = 0;
    UI32_T ifindex, ipaddr;
    UI32_T table_id;
    UI32_T count = 0, nexthop;
    int flags;
    char   line_buffer[MAXLINE];
    struct prefix p;

    while (1)
    {
        IPAL_BACKDOOR_Printf("\r\n----------IPAL UCRoute Configuration ---------------");
        IPAL_BACKDOOR_Printf("\r\n\t 1 : Add Ipv4 Unicast Route Entry");
        IPAL_BACKDOOR_Printf("\r\n\t 2 : Del Ipv4 Unicast Route Entry");
        IPAL_BACKDOOR_Printf("\r\n\t 3 : Show IPv4/IPv6 Unicast Route Table");
        IPAL_BACKDOOR_Printf("\r\n\t 4 : Get best routing interface");
        IPAL_BACKDOOR_Printf("\r\n\t 0 : Return");
        IPAL_BACKDOOR_Printf("\r\n--------------------------------------------------------\r\n");
        IPAL_BACKDOOR_Printf("\r\nEnter Selection:");

        if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        switch (select_value)
        {
            case 1:

                IPAL_BACKDOOR_Printf("\r\nEnter the route Table Id: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    table_id = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter IP Prefix: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_PREFIX_Str2Prefix (line_buffer, &p);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Flags: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    flags = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter nexthop ipaddr (xxx.xxx.xxx.xxx) : ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_INET_Aton((UI8_T *)line_buffer, &nexthop);
                }

                //if(IPAL_AddIPv4Route (table_id,  &p, flags, ifindex, count, nexthop) != IPAL_RESULT_OK)
                {
                    IPAL_BACKDOOR_Printf("\r\nAdd IPV4 Route Fail ret=%d\r\n", errno);
                };
                break;
            case 2:

                IPAL_BACKDOOR_Printf("\r\nEnter the route Table Id: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    table_id = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter IP Prefix: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_PREFIX_Str2Prefix (line_buffer, &p);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Flags: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    flags = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter nexthop : ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                     L_INET_Aton((UI8_T *)line_buffer, &nexthop);
                }
                //if(IPAL_DeleteIPv4Route (table_id, &p, flags, ifindex, count, nexthop)!= IPAL_RESULT_OK)
                {
                    IPAL_BACKDOOR_Printf("\r\nDel IPV4 Route Fail ret=%d\r\n", errno);
                }
                break;
            case 3:
                IPAL_ROUTE_ShowAllIpv4Route();
#if (SYS_CPNT_IPV6 == TRUE)
                IPAL_ROUTE_ShowAllIpv6Route();
#endif
                break;
            case 4:
                {
                    L_INET_AddrIp_T inet_addr, src_addr, nexthop_addr;
                    UI32_T out_ifindex = 0;
                    char buf[64];
                    BACKDOOR_MGR_Printf("\r\nInput dest address (v4/v6):");
                    BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf) - 1);
                    BACKDOOR_MGR_Printf("\r\n");
                    memset(&inet_addr, 0 , sizeof(L_INET_AddrIp_T));
                    memset(&src_addr, 0 , sizeof(L_INET_AddrIp_T));
                    memset(&nexthop_addr, 0 , sizeof(L_INET_AddrIp_T));
                    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                                       buf,
                                                                       (L_INET_Addr_T *)&inet_addr,
                                                                       sizeof(inet_addr)))
                    {
                        BACKDOOR_MGR_Printf("Failed. Address format error.\r\n");
                        break;
                    }
                    if(IPAL_RESULT_OK == IPAL_ROUTE_RouteLookup(&inet_addr, &src_addr, &nexthop_addr, &out_ifindex))
                    {
                        L_INET_InaddrToString((L_INET_Addr_T *) &src_addr, buf, sizeof(buf));
                        BACKDOOR_MGR_Printf("src addr: %s\r\n", buf);
                
                        L_INET_InaddrToString((L_INET_Addr_T *) &nexthop_addr, buf, sizeof(buf));
                        BACKDOOR_MGR_Printf("nexthop_add addr: %s\r\n", buf);
                
                        BACKDOOR_MGR_Printf("out_ifindex: %lu\r\n", out_ifindex);
                    }
                    else
                    {
                        BACKDOOR_MGR_Printf("Failed.\r\n");
                
                    }
                }
                break;
            case 0:
                IPAL_BACKDOOR_Printf("\r\n");
                return;
            default:
                break;
        }
    }
}

static void IPAL_BACKDOOR_InterfaceConfig(void)
{
    #define MAXLINE 255
    UI32_T  select_value = 0, set_value = 0;
    UI32_T ifindex, ipaddr,stat_type,stat_value;
    char   line_buffer[MAXLINE];
    MAC_ADDRESS_T mac;
    UI16_T  flags;
    UI8_T  ifname[IFNAMSIZ] = {0};
    UI32_T mtu,hop_limit, ipv6_mtu;
    UI32_T bandwidth;
    L_INET_AddrIp_T p;
    IPAL_IpAddressInfoEntry_T ipal_ip_info;

    while (1)
    {
        IPAL_BACKDOOR_Printf("\r\n----------IPAL Interface Configuration ---------------");
        IPAL_BACKDOOR_Printf("\r\n\t 1  : Creat Interface ");
        IPAL_BACKDOOR_Printf("\r\n\t 2  : Distory Interface ");
        IPAL_BACKDOOR_Printf("\r\n\t 3  : Get Interface Flag");
        IPAL_BACKDOOR_Printf("\r\n\t 4  : Set Interface Flag");
        IPAL_BACKDOOR_Printf("\r\n\t 5  : Unset Interface Flag.");
        IPAL_BACKDOOR_Printf("\r\n\t 6  : Get Interface MTU");
        IPAL_BACKDOOR_Printf("\r\n\t 7  : Set Interface MTU.");
        IPAL_BACKDOOR_Printf("\r\n\t 8  : Set Interface BandWidth.");
        IPAL_BACKDOOR_Printf("\r\n\t 9  : Set Interface MAC");
        IPAL_BACKDOOR_Printf("\r\n\t 10 : Add Interface IPaddr");
        IPAL_BACKDOOR_Printf("\r\n\t 11 : Del Interface IPaddr");
        IPAL_BACKDOOR_Printf("\r\n\t 12 : Get Interface (first) IPaddr");
        IPAL_BACKDOOR_Printf("\r\n\t 13 : Show All Interface Info");
        IPAL_BACKDOOR_Printf("\r\n\t 14 : Set Interface Hop Limit");
        IPAL_BACKDOOR_Printf("\r\n\t 15 : Set Default Hop Limit");
#if (SYS_CPNT_IPV6 == TRUE)
        IPAL_BACKDOOR_Printf("\r\n\t 16 : Enable IPv6 autoconfig");
        IPAL_BACKDOOR_Printf("\r\n\t 17 : Disable IPv6 autoconfig");
#endif
        IPAL_BACKDOOR_Printf("\r\n\t 18 : Get IPv4 statistic by type");
#if (SYS_CPNT_IPV6 == TRUE)
        IPAL_BACKDOOR_Printf("\r\n\t 19 : Get IPv6 statistic by type");
        IPAL_BACKDOOR_Printf("\r\n\t 20 : Get Next IPv6 Joined Mcast Group");
        IPAL_BACKDOOR_Printf("\r\n\t 21 : Get Next Ipv6 address info");
        IPAL_BACKDOOR_Printf("\r\n\t 22 : Set Ipv6 interface MTU");
        IPAL_BACKDOOR_Printf("\r\n\t 23 : Get Ipv6 interface MTU");
#endif
        IPAL_BACKDOOR_Printf("\r\n\t 0 : Return");
        IPAL_BACKDOOR_Printf("\r\n--------------------------------------------------------\r\n");
        IPAL_BACKDOOR_Printf("\r\nEnter Selection:");

        select_value = 0;
        if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        switch (select_value)
        {
            case 1:
                IPAL_BACKDOOR_Printf("\r\nSelect interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface mac (xx-xx-xx-xx-xx-xx) : ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    AMS_MACADDR_Aton(line_buffer, &mac);
                }

                if (IPAL_RESULT_OK == IPAL_IF_CreateInterface (ifindex, mac.addr))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 2:
                IPAL_BACKDOOR_Printf("\r\nSelect interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                if (IPAL_RESULT_OK == IPAL_IF_DestroyInterface (ifindex))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 3:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                if (IPAL_RESULT_OK == IPAL_IF_GetIfFlags (ifindex, &flags))
                    IPAL_BACKDOOR_Printf("\r\nsuccess: flags = %04x\n",flags);
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 4:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nEnter Flags value: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    flags = atoi(line_buffer);
                }

                if (IPAL_RESULT_OK == IPAL_IF_SetIfFlags (ifindex, flags))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 5:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nEnter Unset Flags value: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    flags = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_IF_UnsetIfFlags (ifindex, flags))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 6:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nEnter Interface Ipaddr: ");
                if (IPAL_RESULT_OK == IPAL_IF_GetIfMtu (ifindex, &mtu))
                    IPAL_BACKDOOR_Printf("\r\nsuccess:mtu=%lu\n",mtu);
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;

            case 7:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nEnter Interface MTU: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    mtu = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_IF_SetIfMtu (ifindex, mtu))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 8:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nEnter BandWidth value : ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    bandwidth = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_IF_SetIfBandwidth(ifindex, bandwidth))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 9:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nenter Interface MAC value(xx-xx-xx-xx-xx-xx): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    AMS_MACADDR_Aton(line_buffer, &mac);
                }
                if (IPAL_RESULT_OK == IPAL_IF_SetIfMac(ifindex, mac.addr))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 10:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nEnter Interface Ipaddr: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC, line_buffer, (L_INET_Addr_T *)&p, sizeof(p));
                }
                if (IPAL_RESULT_OK == IPAL_IF_AddIpAddress (ifindex, &p))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 11:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface Ipaddr: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC, line_buffer, (L_INET_Addr_T *)&p, sizeof(p));
                }
                if (IPAL_RESULT_OK == IPAL_IF_DeleteIpAddress (ifindex, &p))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 12:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_IF_GetIfIpv4Addr(ifindex, &p))
                {
                    if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&p,
                                                                       line_buffer,
                                                                       sizeof(line_buffer)))
                    {
                        IPAL_BACKDOOR_Printf("\r\nsuccess: %s\n", line_buffer);
                    }
                    else IPAL_BACKDOOR_Printf("\r\nsuccess but address error\n");
                }
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 13:
                IPAL_IF_ShowIfInfo();
                break;
#if (SYS_CPNT_IPV6 == TRUE)
            case 14:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }

                IPAL_BACKDOOR_Printf("\r\nEnter Interface Hop Limit: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    hop_limit = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_IF_SetIfHopLimit(ifindex, hop_limit))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 15:
                IPAL_BACKDOOR_Printf("\r\nEnter Default Hop Limit: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    hop_limit = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_IF_SetDefaultHopLimit(hop_limit))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 16:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_IF_EnableIpv6Autoconfig(ifindex))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 17:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_IF_DisableIpv6Autoconfig(ifindex))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */
            case 18:
                IPAL_BACKDOOR_Printf("\r\nSelect Statistic Type: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    stat_type = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_IF_GetIpv4StatisticByType(stat_type,&stat_value))
                {
                    IPAL_BACKDOOR_Printf("\r\nsuccess, value = %lu\n",stat_value);
                }
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
#if (SYS_CPNT_IPV6 == TRUE)
            case 19:
                IPAL_BACKDOOR_Printf("\r\nSelect Statistic Type: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    stat_type = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_IF_GetIpv6StatisticByType(stat_type,&stat_value))
                {
                    IPAL_BACKDOOR_Printf("\r\nsuccess, value = %lu\n",stat_value);
                }
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 20:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nEnter Interface Ipaddr: ");
                memset(&p, 0x0, sizeof(L_INET_AddrIp_T));
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    if (strlen(line_buffer)>0)
                        L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC, line_buffer, (L_INET_Addr_T *)&p, sizeof(p));
                }
                if (IPAL_RESULT_OK == IPAL_IF_GetNextIfJoinIpv6McastAddr(ifindex, &p))
                {
                    if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&p,
                                                                       line_buffer,
                                                                       sizeof(line_buffer)))
                    {
                        IPAL_BACKDOOR_Printf("\r\nsuccess: %s\n", line_buffer);
                    }
                    else IPAL_BACKDOOR_Printf("\r\nsuccess but address error\n");
                }
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 21:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nEnter Interface Ipaddr: ");
                memset(&(ipal_ip_info.ipaddr), 0x0, sizeof(L_INET_AddrIp_T));
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    if (strlen(line_buffer)>0)
                    {
                        L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC, line_buffer,
                            (L_INET_Addr_T *)&ipal_ip_info.ipaddr, sizeof(ipal_ip_info.ipaddr));
                    }
                }
                if (IPAL_RESULT_OK == IPAL_IF_GetNextIfIpv6AddrInfo(ifindex, &ipal_ip_info))
                {
                    if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&ipal_ip_info.ipaddr,
                                                                       line_buffer,
                                                                       sizeof(line_buffer)))
                    {
                        IPAL_BACKDOOR_Printf("\r\nsuccess: %s\n", line_buffer);
                    }
                    else IPAL_BACKDOOR_Printf("\r\nsuccess but address error\n");
                    IPAL_BACKDOOR_Printf("\r\npreferred life time=%lu",ipal_ip_info.preferred_lft);
                    IPAL_BACKDOOR_Printf("\r\nvalid life time=%lu",ipal_ip_info.valid_lft);
                    IPAL_BACKDOOR_Printf("\r\nscope=%lu",ipal_ip_info.scope);
                    IPAL_BACKDOOR_Printf("\r\ntentative=%u",ipal_ip_info.tentative);
                    IPAL_BACKDOOR_Printf("\r\ndeprecated=%u",ipal_ip_info.deprecated);
                    IPAL_BACKDOOR_Printf("\r\npermanent=%u\n",ipal_ip_info.permanent);
                }
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;

            case 22:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nInput ipv6 mtu: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ipv6_mtu = atoi(line_buffer);
                }

                if (IPAL_RESULT_OK == IPAL_IF_SetIpv6Mtu(ifindex, ipv6_mtu))
                {
                    IPAL_BACKDOOR_Printf("\r\nok\r\n");
                }
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;

            case 23:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_IF_GetIpv6Mtu(ifindex, &ipv6_mtu))
                {
                    IPAL_BACKDOOR_Printf("\r\nipv6 interface mtu=%ld\n",ipv6_mtu);
                }
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;

#endif /* #if (SYS_CPNT_IPV6 == TRUE) */
            case 0:
                IPAL_BACKDOOR_Printf("\r\n");
                return;
            default:
                break;
        }
    }
}

static void IPAL_BACKDOOR_IcmpConfig(void)
{
    #define MAXLINE 255
    UI32_T  select_value = 0;
    UI32_T ifindex, set_value = 0;
    BOOL_T mbit,obit;
    char   line_buffer[MAXLINE];
    UI8_T  ifname[IFNAMSIZ] = {0};

    while (1)
    {
        IPAL_BACKDOOR_Printf("\r\n----------IPAL Icmp Configuration ---------------");
#if (SYS_CPNT_IPV6 == TRUE)
        IPAL_BACKDOOR_Printf("\r\n\t 1 : IPAL_ICMP_SetIpv6IcmpRateLimit ");
        IPAL_BACKDOOR_Printf("\r\n\t 2 : IPAL_ICMP_SetIpv6NeighDefaultRetransTime ");
        IPAL_BACKDOOR_Printf("\r\n\t 3 : IPAL_ICMP_SetIpv6NeighRetransTime");
        IPAL_BACKDOOR_Printf("\r\n\t 4 : IPAL_ICMP_SetIpv6NeighDefaultReachableTime");
        IPAL_BACKDOOR_Printf("\r\n\t 5 : IPAL_ICMP_SetIpv6NeighReachableTime");
        IPAL_BACKDOOR_Printf("\r\n\t 6 : IPAL_ICMP_SetIpv6DadTransmits");
        IPAL_BACKDOOR_Printf("\r\n\t 7 : IPAL_ICMP_GetRaMoBits");
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */
        IPAL_BACKDOOR_Printf("\r\n\t 0 : Return");
        IPAL_BACKDOOR_Printf("\r\n--------------------------------------------------------\r\n");
        IPAL_BACKDOOR_Printf("\r\nEnter Selection:");

        if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        switch (select_value)
        {
#if (SYS_CPNT_IPV6 == TRUE)
            case 1:
                IPAL_BACKDOOR_Printf("\r\nvalue: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    set_value = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_ICMP_SetIpv6IcmpRateLimit(set_value))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 2:
                IPAL_BACKDOOR_Printf("\r\nvalue: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    set_value = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_ICMP_SetIpv6NeighDefaultRetransTime(set_value))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 3:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nvalue: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    set_value = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_ICMP_SetIpv6NeighRetransTime(ifindex,set_value))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 4:
                IPAL_BACKDOOR_Printf("\r\nvalue: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    set_value = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_ICMP_SetIpv6NeighDefaultReachableTime(set_value))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 5:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nvalue: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    set_value = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_ICMP_SetIpv6NeighReachableTime(ifindex, set_value))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 6:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                IPAL_BACKDOOR_Printf("\r\nvalue: ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    set_value = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_ICMP_SetIpv6DadTransmits(ifindex,set_value))
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 7:
                IPAL_BACKDOOR_Printf("\r\nSelect Interface (ifindex): ");
                if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ifindex = atoi(line_buffer);
                }
                if (IPAL_RESULT_OK == IPAL_ICMP_GetRaMoBits(ifindex, &mbit, &obit))
                {
                    IPAL_BACKDOOR_Printf("\r\nsuccess, mbit=%u, obit=%u\n",mbit,obit);
                }
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */
            case 0:
                IPAL_BACKDOOR_Printf("\r\n");
                return;
            default:
                break;
        }
    }
}

static void IPAL_BACKDOOR_RouteConfig(void)
{
    #define MAXLINE 255
    UI32_T  select_value = 0;
    UI32_T ifindex, set_value = 0;
    BOOL_T mbit,obit;
    char   line_buffer[MAXLINE];
    UI8_T  ifname[IFNAMSIZ] = {0};
    I32_T  ip_forward;
    IPAL_PmtuEntry_T pmtu_entry;
    UI32_T           pmtu_entry_num;

    while (1)
    {
        IPAL_BACKDOOR_Printf("\r\n----------IPAL Route Configuration ---------------");
        IPAL_BACKDOOR_Printf("\r\n\t 1 : IPAL_ROUTE_EnableIpv4Forwarding ");
        IPAL_BACKDOOR_Printf("\r\n\t 2 : IPAL_ROUTE_DisableIpv4Forwarding ");
        IPAL_BACKDOOR_Printf("\r\n\t 3 : IPAL_ROUTE_GetIpv4Forwarding");
#if (SYS_CPNT_IPV6 == TRUE)
        IPAL_BACKDOOR_Printf("\r\n\t 4 : IPAL_ROUTE_EnableIpv6Forwarding");
        IPAL_BACKDOOR_Printf("\r\n\t 5 : IPAL_ROUTE_DisableIpv6Forwarding");
        IPAL_BACKDOOR_Printf("\r\n\t 6 : IPAL_ROUTE_GetIpv6Forwarding");
        IPAL_BACKDOOR_Printf("\r\n\t 7 : IPAL_ROUTE_GetNextNPmtu");
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */
        IPAL_BACKDOOR_Printf("\r\n\t 0 : Return");
        IPAL_BACKDOOR_Printf("\r\n--------------------------------------------------------\r\n");
        IPAL_BACKDOOR_Printf("\r\nEnter Selection:");

        if (IPAL_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        switch (select_value)
        {
            case 1:
                if (IPAL_RESULT_OK == IPAL_ROUTE_EnableIpv4Forwarding())
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 2:
                if (IPAL_RESULT_OK == IPAL_ROUTE_DisableIpv4Forwarding())
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 3:
                if (IPAL_RESULT_OK == IPAL_ROUTE_GetIpv4Forwarding(&ip_forward))
                {
                    IPAL_BACKDOOR_Printf("\r\nsuccess: %d\n",ip_forward);
                }
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
#if (SYS_CPNT_IPV6 == TRUE)
            case 4:
                if (IPAL_RESULT_OK == IPAL_ROUTE_EnableIpv6Forwarding())
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 5:
                if (IPAL_RESULT_OK == IPAL_ROUTE_DisableIpv6Forwarding())
                    IPAL_BACKDOOR_Printf("\r\nsuccess\n");
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 6:
                if (IPAL_RESULT_OK == IPAL_ROUTE_GetIpv6Forwarding(&ip_forward))
                {
                    IPAL_BACKDOOR_Printf("\r\nsuccess: %d\n",ip_forward);
                }
                else IPAL_BACKDOOR_Printf("\r\nfail\n");
                break;
            case 7:
                memset(&pmtu_entry, 0x0, sizeof(IPAL_PmtuEntry_T));
                while (TRUE)
                {
                    pmtu_entry_num = 1;
                    if (IPAL_RESULT_OK == IPAL_ROUTE_GetNextNPmtuEntry(&pmtu_entry, &pmtu_entry_num))
                    {
                        if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&pmtu_entry.destip,
                                                                           line_buffer,
                                                                           sizeof(line_buffer)))
                        {
                            IPAL_BACKDOOR_Printf("\r\nsuccess: %s\n", line_buffer);
                        }
                        else IPAL_BACKDOOR_Printf("\r\nsuccess but address error\n");
                        IPAL_BACKDOOR_Printf("\r\nsuccess: pmtu=%lu since_time=%lu\n",pmtu_entry.pmtu,pmtu_entry.since_time);
                    }
                    else
                    {
                        IPAL_BACKDOOR_Printf("\r\nfail\n");
                        break;
                    }
                }
                break;
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */
            case 0:
                IPAL_BACKDOOR_Printf("\r\n");
                return;
            default:
                break;
        }
    }
}

