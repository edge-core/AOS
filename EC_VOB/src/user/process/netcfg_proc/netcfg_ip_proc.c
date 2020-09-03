#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include "netcfg_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "leaf_es3626a.h"
#include "l_inet.h"
#include "ip_lib.h"
#include "vlan_lib.h"
#include "netcfg_pmgr_ip.h"
#include "netcfg_netdevice.h"
#include "netcfg_pom_ip.h"

#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#define debug_printf printf
#else
#define debug_printf(...) do { } while (0)
#endif

static int NETCFG_IP_PROC_AtoIp(char *s, UI8_T *ip);
//static UI32_T NETCFG_IP_PROC_MaskToCidr(UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN]);

static int NETCFG_IP_PROC_SetCraft_Address(UI32_T row_status);
static int NETCFG_IP_PROC_SetCraft_v6Address(UI32_T row_status);
static int NETCFG_IP_PROC_Set_Address(UI32_T ifindex, UI32_T row_status);
static int NETCFG_IP_PROC_Set_v6Address(UI32_T ifindex, UI32_T row_status);


/* environment variable examples on spawning usermode helper function
IF_PID=1053
IF_CMD=35094
IF_COMM=ifconfig
IF_NAME=lo
IF_FAMI=2  ----> AF_INET:2 AF_INET6: 10
IF_ADDR=192.168.1.2
IF_MASK=255.255.255.0  -> store prefix length when IPv6
*/
static char *ip_addr, *netmask, *if_name;

/*********************************************************************/
/* Convert string s (xxx.xxx.xxx.xxx) to IP address (4-byte object). */
/*                                                                   */
/* Return: 1 => success, result put in ip                            */
/*         0 => fail, content of ip undetermined                     */
/*********************************************************************/

/* Copy from CLI_LIB_AtoIp(char *s, UI8_T *ip)
 */
static int NETCFG_IP_PROC_AtoIp(char *s, UI8_T *ip)
{
    char  token[20];
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
            if (strlen(token) < 1 || strlen(token) > 3 ||
                atoi(token) < 0 || atoi(token) > 255)
            {
                return 0;
            }
            else if (k >= 4)
            {
                return 0;
            }
            else
            {
                temp[k++] =(UI8_T)atoi(token);
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
    if (strlen(token) < 1 || strlen(token) > 3 ||
        atoi(token) < 0 || atoi(token) > 255)
    {
        return 0;
    }
    else if (k != 3)
    {
        return 0;
    }

    temp[k]=(UI8_T)atoi(token);

/*    ip[0] = temp[3];
    ip[1] = temp[2];
    ip[2] = temp[1];
    ip[3] = temp[0];*/

    ip[0] = temp[0];
    ip[1] = temp[1];
    ip[2] = temp[2];
    ip[3] = temp[3];

    return 1;
}

static int NETCFG_IP_PROC_SetCraft_Address(UI32_T row_status)
{
//    UI32_T uint_ipMask=0;
    UI8_T  byte_mask[SYS_ADPT_IPV4_ADDR_LEN];
    NETCFG_TYPE_CraftInetAddress_T craft_addr;
    UI32_T ret;

    memset(&craft_addr, 0, sizeof(craft_addr));
    craft_addr.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;

    if (row_status == VAL_netConfigStatus_2_destroy)
    {
        if (NETCFG_TYPE_OK == NETCFG_POM_IP_GetCraftInterfaceInetAddress(&craft_addr))
        {
#if 0
            /* the following is for debug too */
            {
                char   line_buffer[256];
                if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&(craft_addr.addr),
                                                                     line_buffer,
                                                                     sizeof(line_buffer)))
                {
                    printf("\r\n%s:got OM IP address on CRAFT port: %s\n",__FUNCTION__, line_buffer);
                }
                else
                    printf("\r\n%s:got OM IP address on CRAFT port error\n",__FUNCTION__);
            }
#endif
            craft_addr.row_status = row_status;
            craft_addr.by_user_mode_helper_func = 1;
            if (NETCFG_TYPE_OK == NETCFG_PMGR_IP_SetCraftInterfaceInetAddress(&craft_addr))
            {
                debug_printf("Successfully destroy the IP address of craft port.\r\n");
                return 0;
            }
        }
        else
            return 0;
        debug_printf("Failed to destroy the IP address of craft port.\r\n");
        return EFAULT;
    }

    if (NETCFG_IP_PROC_AtoIp(netmask, byte_mask) == 0 || IP_LIB_IsValidNetworkMask(byte_mask) == FALSE )
    {
        debug_printf("Invalid Netmask address - %s\r\n", netmask);
        return EFAULT;
    }
        /* format: 192.168.1.1 255.255.255.0 */

        /* convert to byte array format */
//        IP_LIB_UI32toArray(uint_ipMask, byte_mask);
    if (ip_addr == NULL)
    {
#if 0
        char default_ip[]="192.168.1.51";
        ip_addr=&default_ip[0];
        L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR, ip_addr,
                              (L_INET_Addr_T *) &craft_addr.addr,
                               sizeof(craft_addr.addr));
#endif
        /* apply to current IP
         */
        if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetCraftInterfaceInetAddress(&craft_addr))
        {
            debug_printf("can't get IP address from OM - %s %d\r\n", __FUNCTION__, __LINE__);
            return EFAULT;
        }
#if 0
        /* the following is for debug too */
        {
            char   line_buffer[256];
            if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&(craft_addr.addr),
                                                                 line_buffer,
                                                                 sizeof(line_buffer)))
            {
                printf("\r\n%s:got OM IP address on CRAFT port: %s\n",__FUNCTION__, line_buffer);
            }
            else
                printf("\r\n%s:got OM IP address on CRAFT port error\n",__FUNCTION__);
        }
#endif
        craft_addr.row_status = row_status;
    }
    else
    {
        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                           ip_addr,
                                                           (L_INET_Addr_T *) &craft_addr.addr,
                                                           sizeof(craft_addr.addr)))
        {
            debug_printf("Invalid IP address:%s - %s %d\r\n", ip_addr, __FUNCTION__, __LINE__);
            return EFAULT;
        }
    }

    craft_addr.addr.preflen = IP_LIB_MaskToCidr(byte_mask);
    debug_printf("craft_addr.addr.preflen=%d\r\n", craft_addr.addr.preflen);

#if 0
    else
    {
        /* format: 192.168.1.1/24 */
        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                           arg[0],
                                                           (L_INET_Addr_T *) &craft_addr.addr,
                                                           sizeof(craft_addr.addr)))
        {
            CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
            return CLI_NO_ERROR;
        }
    }
#endif

    ret = IP_LIB_IsValidForIpConfig(craft_addr.addr.addr, byte_mask);
    switch(ret)
    {
        case IP_LIB_INVALID_IP:
        case IP_LIB_INVALID_IP_LOOPBACK_IP:
        case IP_LIB_INVALID_IP_ZERO_NETWORK:
        case IP_LIB_INVALID_IP_BROADCAST_IP:
        case IP_LIB_INVALID_IP_IN_CLASS_D:
        case IP_LIB_INVALID_IP_IN_CLASS_E:
            debug_printf("Invalid IP address - %s %d\r\n", __FUNCTION__, __LINE__);
            return EFAULT;
            break;
        case IP_LIB_INVALID_IP_SUBNET_NETWORK_ID:
            debug_printf("Invalid IP address. Can't be network ID.\r\n");
            return EFAULT;
            break;
        case IP_LIB_INVALID_IP_SUBNET_BROADCAST_ADDR:

            debug_printf("Invalid IP address. Can't be network b'cast IP.\r\n");
            return EFAULT;

        default:
            break;
    }

    craft_addr.row_status = row_status;
    craft_addr.by_user_mode_helper_func = 1;
    /* set craft interface's ip address
     */
    ret = NETCFG_PMGR_IP_SetCraftInterfaceInetAddress(&craft_addr);

    switch(ret)
    {
        case NETCFG_TYPE_OK:
            break;
        case NETCFG_TYPE_ERR_ROUTE_NEXTHOP_CONFLICT_ON_NORMAL_INT_AND_CRAFT_INT:
            debug_printf("Failed. Due to a static route's nexthop conflicts on normal interface and craft interface.\r\n");
            return EFAULT;
        case NETCFG_TYPE_FAIL:
        default:
            debug_printf("Failed to set/add IP address of craft port.\r\n");
            return EFAULT;
    }
    return 0;
}

static int NETCFG_IP_PROC_SetCraft_v6Address(UI32_T row_status)
{
    NETCFG_TYPE_CraftInetAddress_T craft_addr;
    UI32_T ret;
    char ip_prefix[L_INET_MAX_IP6ADDR_STR_LEN+1];

    memset(&craft_addr, 0, sizeof(craft_addr));
    craft_addr.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;

    if (ip_addr == NULL)
    {
        return EFAULT;
    }
    else
    {
        if (strlen(netmask)>3) /* we use netmask to store prefix length for ipv6 */
        {
            debug_printf("Invalid prefix length: %s\r\n", netmask);
            return EFAULT;
        }
        sprintf(ip_prefix,"%s/%s", ip_addr, netmask);
        debug_printf("Got IPv6 address:%s\r\n", ip_prefix);

        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_ADDR_RAW,
                                                           ip_prefix,
                                                           (L_INET_Addr_T *) &craft_addr.addr,
                                                           sizeof(craft_addr.addr)))
        {
            debug_printf("Invalid IPv6 address:%s - %s %d\r\n", ip_prefix, __FUNCTION__, __LINE__);
            return EFAULT;
        }
    }

    if (L_INET_ADDR_IS_IPV6_LINK_LOCAL_UNICAST(craft_addr.addr.addr))
    {
        craft_addr.ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL;
        if (row_status == VAL_netConfigStatus_2_createAndGo)
            craft_addr.addr.preflen = 64;
        craft_addr.addr.zoneid = SYS_ADPT_CRAFT_INTERFACE_IFINDEX; /* assign zoneid */
    }
    else
        craft_addr.ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL;

    craft_addr.ipv6_addr_config_type = NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_MANUAL;

    if (row_status == VAL_netConfigStatus_2_destroy && craft_addr.addr.type == L_INET_ADDR_TYPE_IPV6Z)
        craft_addr.addr.preflen = 64;

    craft_addr.row_status = row_status;
    craft_addr.by_user_mode_helper_func = 1;
    /* set craft interface's ip address
     */
    ret = NETCFG_PMGR_IP_SetCraftInterfaceInetAddress(&craft_addr);

    switch(ret)
    {
        case NETCFG_TYPE_OK:
            break;
        case NETCFG_TYPE_INVALID_IPV6_UNSPECIFIED:
            debug_printf("Unspecified address is not allowed.\r\n");
            return EFAULT;
        case NETCFG_TYPE_INVALID_IPV6_LOOPBACK:
            debug_printf("Loopback address is not allowed.\r\n");
            return EFAULT;
        case NETCFG_TYPE_INVALID_IPV6_MULTICAST:
            debug_printf("IPv6 multicast address is not allowed.\r\n");
            return EFAULT;
        case NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED:
            debug_printf("Failed to add IPv6 address. Overlapped with existing address.\r\n");
            return EFAULT;
        case NETCFG_TYPE_ENTRY_NOT_EXIST: /* may happen in detroy case */
            debug_printf("Address not found.\r\n");
            return EFAULT;
        default:
            /* unknown error */
            debug_printf("Failed to add or delete IPv6 address.\r\n");
            return EFAULT;
    }
    return 0;
}


static int NETCFG_IP_PROC_Set_Address(UI32_T ifindex, UI32_T row_status)
{
    NETCFG_TYPE_InetRifConfig_T rif_config;
    UI8_T  byte_mask[SYS_ADPT_IPV4_ADDR_LEN];
    UI32_T ret;

    debug_printf("NETCFG_IP_PROC_Set_Address on ifindex %lu\r\n", ifindex);

    memset(&rif_config, 0, sizeof(rif_config));
    rif_config.ifindex = ifindex;

    if (row_status == VAL_netConfigStatus_2_destroy)
    {
        /* we must destroy secondary rif first
         */
        while(NETCFG_TYPE_OK == NETCFG_POM_IP_GetNextSecondaryRifFromInterface(&rif_config))
        {
            rif_config.row_status = row_status;
            if(NETCFG_TYPE_OK != NETCFG_PMGR_IP_SetInetRif(&rif_config,NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB))
            {
                debug_printf("Failed to destory IP address on interface %s\r\n", if_name);
                return EFAULT;
            }
        }

        /* Destroy primary rif at last
         */
        memset(&rif_config, 0, sizeof(rif_config));
        rif_config.ifindex = ifindex;

        if(NETCFG_TYPE_OK == NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config))
        {
            rif_config.row_status = row_status;
            if(NETCFG_TYPE_OK != NETCFG_PMGR_IP_SetInetRif(&rif_config,NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB))
            {
                debug_printf("Failed to destory IP address on interface %s\r\n", if_name);
                return EFAULT;
            }
        }
        return 0;
    }

    if (NETCFG_IP_PROC_AtoIp(netmask, byte_mask) == 0 || IP_LIB_IsValidNetworkMask(byte_mask) == FALSE )
    {
        debug_printf("Invalid Netmask address - %s\r\n", netmask);
        return EFAULT;
    }
    if (ip_addr == NULL)
    {
        /* apply to current primary IP
         */
        if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config))
        {
            debug_printf("can't get IP address from OM - %s %d\r\n", __FUNCTION__, __LINE__);
            return EFAULT;
        }
 #if 0
        /* the following is for debug too */
        {
            char   line_buffer[256];
            if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&(rif_config.addr),
                                                                 line_buffer,
                                                                 sizeof(line_buffer)))
            {
                printf("\r\n%s:got OM IP address on interface: %s\n",__FUNCTION__, line_buffer);
            }
            else
                printf("\r\n%s:got OM IP address on interface error\n",__FUNCTION__);
        }
 #endif
    }
    else
    {
        /* format: 192.168.1.1 255.255.255.0 */
        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                           ip_addr,
                                                           (L_INET_Addr_T *) &rif_config.addr,
                                                           sizeof(rif_config.addr)))
        {
            debug_printf("Invalid IP address:%s - %s %d\r\n", ip_addr, __FUNCTION__, __LINE__);
            return EFAULT;
        }
    }
    /* kernel's secondary address is on the same subnet;
     * AOS's is on the different subnet
     * so we can't support secondary here
     */
    rif_config.ipv4_role = NETCFG_TYPE_MODE_PRIMARY;
    rif_config.row_status = row_status;
    rif_config.addr.preflen = IP_LIB_MaskToCidr(byte_mask);
    debug_printf("rif_config.addr.preflen=%d\r\n", rif_config.addr.preflen);


    ret = IP_LIB_IsValidForIpConfig(rif_config.addr.addr, byte_mask);
    switch(ret)
    {
        case IP_LIB_INVALID_IP:
        case IP_LIB_INVALID_IP_LOOPBACK_IP:
        case IP_LIB_INVALID_IP_ZERO_NETWORK:
        case IP_LIB_INVALID_IP_BROADCAST_IP:
        case IP_LIB_INVALID_IP_IN_CLASS_D:
        case IP_LIB_INVALID_IP_IN_CLASS_E:
            debug_printf("Invalid IP address - %s %d\r\n", __FUNCTION__, __LINE__);
            return EFAULT;
            break;
       case IP_LIB_INVALID_IP_SUBNET_NETWORK_ID:
            debug_printf("Invalid IP address. Can't be network ID.\r\n");
            return EFAULT;
            break;
        case IP_LIB_INVALID_IP_SUBNET_BROADCAST_ADDR:
            {
                UI32_T lo_id;

                if (TRUE != IP_LIB_ConvertLoopbackIfindexToId(ifindex, &lo_id))
                {
                    debug_printf("Invalid IP address. Can't be network b'cast IP.\r\n");
                    return EFAULT;
                }
            }
        default:
            break;
    }

    ret = NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
    if(ret != NETCFG_TYPE_OK)
    {
        debug_printf("Failed to set IP address on interface %s\r\n", if_name);
        return EFAULT;
    }
    return 0;
}

static int NETCFG_IP_PROC_Set_v6Address(UI32_T ifindex, UI32_T row_status)
{
    NETCFG_TYPE_InetRifConfig_T rif_config;
    char ip_prefix[L_INET_MAX_IP6ADDR_STR_LEN+1];
    UI32_T ret;

    debug_printf("NETCFG_IP_PROC_Set_v6Address on ifindex %lu\r\n", ifindex);

    memset(&rif_config, 0, sizeof(rif_config));
    rif_config.ifindex = ifindex;

    if (ip_addr == NULL)
    {
        return EFAULT;
    }
    else
    {
        if (strlen(netmask)>3) /* we use netmask to store prefix length for ipv6 */
        {
            debug_printf("Invalid prefix length: %s\r\n", netmask);
            return EFAULT;
        }
        sprintf(ip_prefix,"%s/%s", ip_addr, netmask);
        debug_printf("Got IPv6 address:%s\r\n", ip_prefix);
        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_ADDR_RAW,
                                                           ip_prefix,
                                                           (L_INET_Addr_T *) &rif_config.addr,
                                                           sizeof(rif_config.addr)))
        {
            debug_printf("Invalid IPv6 address:%s - %s %d\r\n", ip_prefix, __FUNCTION__, __LINE__);
            return EFAULT;
        }
    }

    if (L_INET_ADDR_IS_IPV6_LINK_LOCAL_UNICAST(rif_config.addr.addr))
    {
        rif_config.ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL;
        if (row_status == VAL_netConfigStatus_2_createAndGo)
            rif_config.addr.preflen = 64;
        rif_config.addr.zoneid = ifindex - SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1;/* assign zoneid */
    }
    else
        rif_config.ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL;

    rif_config.ipv6_addr_config_type = NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_MANUAL;

    if (row_status == VAL_netConfigStatus_2_destroy && rif_config.addr.type == L_INET_ADDR_TYPE_IPV6Z)
        rif_config.addr.preflen = 64;

    rif_config.row_status = row_status;

    ret = NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
    if(ret != NETCFG_TYPE_OK)
    {
        debug_printf("Failed to set/add IPv6 address on interface %s\r\n", if_name);
        return EFAULT;
    }
    return 0;
}

/*
 * error  -> return EFAULT (14)
 * success-> return 0
 * let kernel continue to run original code -> return EINVAL (22)
 */
int main(int argc, char *argv[])
{
    UI32_T ifindex, row_status;
    char *ip_family, *cmd;
    char default_netmask[]="255.255.255.0";
    int addr_family, cmd_code=0;

    debug_printf("Start running %s\r\n", argv[0]);

    /* get the needed config from environment
     */
    if_name = getenv("IF_NAME");

    if (if_name == NULL)
        return EFAULT;
    debug_printf("IF_NAME=%s\r\n", if_name);

    netmask = getenv("IF_MASK");
    if (netmask == NULL)
        netmask=&default_netmask[0];
    debug_printf("IF_MASK=%s\r\n", netmask);

    NETCFG_PMGR_IP_InitiateProcessResource();
    NETCFG_POM_IP_InitiateProcessResource();

    cmd = getenv("IF_CMD");
    if (cmd != NULL)
        cmd_code = atoi(cmd);

    ip_addr = getenv("IF_ADDR");
    if (ip_addr != NULL)
    {
        debug_printf("IF_ADDR=%s\r\n", ip_addr);
        if (strlen(ip_addr) == 0)
            ip_addr = NULL;
    }

    if (ip_addr != NULL && (strcmp(ip_addr, "0.0.0.0") == 0 || cmd_code == SIOCDIFADDR))
        row_status = VAL_netConfigStatus_2_destroy;
    else
        row_status = VAL_netConfigStatus_2_createAndGo;

    ip_family = getenv("IF_FAMI");
    if (ip_family == NULL)
    {
        debug_printf("Unsupported Address Family\r\n");
        return EFAULT;
    }

    addr_family = atoi(ip_family);
    if (addr_family == AF_INET) /* ipv4 */
    {
        /* get ifindex
         */
        if (NETCFG_NETDEVICE_IfnameToIfindex(if_name, &ifindex) != TRUE)
        {
            debug_printf("Invalid Interface: %s\r\n", if_name);
            return EINVAL; /* for kernel to continue to run the original code */
        }
        debug_printf("IfIndex=%lu\r\n", ifindex);
        if (ifindex == SYS_ADPT_CRAFT_INTERFACE_IFINDEX)
            return NETCFG_IP_PROC_SetCraft_Address(row_status);
        else
            return NETCFG_IP_PROC_Set_Address(ifindex, row_status);
    }
    else if (addr_family == AF_INET6) /* ipv6 */
    {
        ifindex = atoi(if_name);
        if (ifindex == SYS_ADPT_CRAFT_INTERFACE_IFINDEX)
            return NETCFG_IP_PROC_SetCraft_v6Address(row_status);
        else if (ifindex == 1) /* for looback, not support set ipv6 address in netcfg */
            return EINVAL; /* for kernel to continue to run the original code */
        else
            return NETCFG_IP_PROC_Set_v6Address(ifindex, row_status);
    }
    debug_printf("Unsupported Address Family\r\n");
    return EINVAL; /* for kernel to continue to run the original code */
}

/* the following are dummy APIs for passing linker
 */
BOOL_T SYSLOG_PMGR_AddEntrySync(void *syslog_entry)
{
    return TRUE;
}

