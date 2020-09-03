#include "sys_adpt.h"
#include "cli_api.h"
#include "cli_api_l2_ip.h"
#include "netcfg_type.h"
#include "netcfg_pmgr_ip.h"
#include "netcfg_pom_ip.h"
#include "netcfg_pmgr_route.h"
#include "netcfg_pmgr_nd.h"
#include "netcfg_pom_nd.h"
#include "netcfg_pom_route.h"
#include "iml_pmgr.h"
#include "ip_lib.h"
#include "dhcp_pmgr.h"
#include <stdio.h>

#include "ipal_if.h"
#include "ipal_types.h"

/***************************************<<IP FOR L2>>****************************************/

/*fuzhimin, 20090106*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
#include <netinet/in.h> 
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#include <unistd.h>
#endif
/*fuzhimin,20090106,end*/

/*fuzhimin,20090212*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
static int if_ioctl (int request, caddr_t buffer);
static int setEth0IpAddr(char*devName,int unit,int ipaddr,int netmask);
static int delEth0IpAddr(char*devName,int unit);

/* call ioctl system call */
static int if_ioctl (int request, caddr_t buffer)
{
    int sock;
    int ret = 0;
    
    sock = socket (AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        /*printf ("if_ioctl: failed to create a socket\n");*/
        return (-1/*ERROR*/);
    }
    
    ret = ioctl (sock, request, buffer);
    if (ret < 0)
    {
        /*printf ("if_ioctl: failed to ioctl\n");*/
        close (sock);
        return (-1/*ERROR*/);
    }
    close (sock);
    return 0;
}


/*******************************************************************************
*
*setEth0IpAddr
*/

static int setEth0IpAddr
(
    char *      devName,                /* device name e.g. "eth" */
    int         unit,   /* unit number */
    int         ipaddr,                   /* target ip address */
    int         netmask               /* subnet mask */
)
{
    char ifname [20];
    int s;
    struct ifreq ifr;
    struct sockaddr_in addrTmp;
    int ret;
    struct sockaddr_in mask;
 
    /*
     * Do nothing if another device is already configured or an
     * error was detected in the boot parameters.
     */

    if (devName == NULL || devName[0] == '\0' )
    {
        /*printf("setEth0IpAddr: Invalid Argument\n");*/
        return (-1);
    }

    /* build interface name */

    bzero (ifname, sizeof (ifname));
    sprintf (ifname, "%s%d", devName, unit);
    strncpy (ifr.ifr_name, ifname, IFNAMSIZ);

    /*set IFADDRESS*/
    addrTmp.sin_addr.s_addr = ipaddr;
    addrTmp.sin_family = AF_INET;
    memcpy (&ifr.ifr_addr, &addrTmp, sizeof (struct sockaddr_in));
    ret = if_ioctl (SIOCSIFADDR, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("setEth0IpAddr: Failed SIOCSIFADDR for %s\n", ifname);*/
        return (-1);
    }

    /*set NETMASK*/
    mask.sin_addr.s_addr = netmask;
    mask.sin_family = AF_INET;
    memcpy (&ifr.ifr_netmask, &mask, sizeof (struct sockaddr_in));
    ret = if_ioctl (SIOCSIFNETMASK, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("setEth0IpAddr: Failed SIOCSIFNETMASK for %s\n", ifname);*/
        return (-1);
    }
#if 0    
    /* set subnet mask, if any specified */

    if (netmask &&
 netIoctl (ifname, SIOCSIFNETMASK, htonl(netmask)) != 0)
 {
 printf ("usrNetBootConfig: Failed SIOCSIFNETMASK for %s\n", ifname);
 return (-1);
 }
    
    /* set inet addr */

    ipaddr = inet_addr(addr);      /* htonl is done inside inet_addr */
    if (netIoctl(ifname, SIOCSIFADDR, ipaddr) != 0)
 {
 printf("usrNetBootConfig: Failed SIOCSIFADDR for %s\n", ifname);
 return (-1);
 }
#endif
  
    /* Set IFF_UP */
    if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        /*printf ("setEth0IpAddr: failed to create a socket\n");*/
        return (-1/*ERROR*/);
    }
    
    bzero ((caddr_t)&ifr, sizeof (ifr));
    strncpy (ifr.ifr_name, ifname, IFNAMSIZ);
    
    if(ioctl(s, (int)SIOCGIFFLAGS, (int)&ifr) != 0) 
    {
        /*printf ("setEth0IpAddr: ioctl (SIOCGIFFLAGS)\n");*/
        close (s);
        return (-1);
    }
    ifr.ifr_flags |= IFF_UP;
    if(ioctl (s, (int)SIOCSIFFLAGS, (int)&ifr) != 0) 
    {
        /*printf("setEth0IpAddr: ioctl (SIOCSIFFLAGS)\n");*/
        close (s);
        return (-1);
    }
    close (s);
    
    return (0);
}



/*******************************************************************************
*
*delEth0IpAddr
*/

static int delEth0IpAddr
(
    char *      devName,                /* device name e.g. "eth" */
    int         unit

)
{
    char ifname [20];
    struct ifreq ifr;
    int ret;
 
    /*
     * Do nothing if another device is already configured or an
     * error was detected in the boot parameters.
     */

    if (devName == NULL || devName[0] == '\0' )
    {
        /*printf("delEth0IpAddr: Invalid Argument\n");*/
        return (-1);
    }

    /* build interface name */

    bzero (ifname, sizeof (ifname));
    sprintf (ifname, "%s%d", devName, unit);
    strncpy (ifr.ifr_name, ifname, IFNAMSIZ);
 
    ret = if_ioctl (SIOCGIFFLAGS, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("delEth0IpAddr: Failed SIOCGIFFLAGS for %s\n", ifname);*/
        return (-1);
    }
    ifr.ifr_flags &= ~IFF_UP;
    ret = if_ioctl (SIOCSIFFLAGS, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("delEth0IpAddr: Failed SIOCSIFFLAGS for %s\n", ifname);*/
        return (-1);
    }
    
    return (0);
}



/*******************************************************************************
*
*getEth0IpAddr
*/
int getEth0IpAddr
(
    char *      devName,                /* device name e.g. "eth" */
    int         unit,
    int *  ipAddr,
    int *  netmask,
    int*   flag
)
{
    char ifname [20];
    struct ifreq ifr;
    int ret;
 
    /*
     * Do nothing if another device is already configured or an
     * error was detected in the boot parameters.
     */

    if (devName == NULL || devName[0] == '\0' 
   || ipAddr == NULL)
    {
        /*printf("delEth0IpAddr: Invalid Argument\n");*/
        return (-1);
    }

    /* build interface name */
    bzero (ifname, sizeof (ifname));
    bzero ((caddr_t)&ifr, sizeof (ifr));
    sprintf (ifname, "%s%d", devName, unit);
    strncpy (ifr.ifr_name, ifname, IFNAMSIZ);

    ret = if_ioctl (SIOCGIFFLAGS, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("delEth0IpAddr: Failed SIOCGIFADDR for %s\n", ifname);*/
        return (-1);
    }
    *flag = ifr.ifr_flags;
 
    ret = if_ioctl (SIOCGIFADDR, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("delEth0IpAddr: Failed SIOCGIFADDR for %s\n", ifname);*/
        return (-1);
    }
    *ipAddr = ((struct sockaddr_in*)(&(ifr.ifr_addr)))->sin_addr.s_addr;

    ret = if_ioctl (SIOCGIFNETMASK, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("delEth0IpAddr: Failed SIOCGIFNETMASK for %s\n", ifname);*/
        return (-1);
    }
    *netmask = ((struct sockaddr_in*)(&(ifr.ifr_netmask)))->sin_addr.s_addr;

    return (0);
}
#endif

/*fuzhimin,20090212,end*/


/*fuzhimin,20090106*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
UI32_T CLI_API_Ip_Address_Eth0(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
      UI32_T uint_ipAddress=0;
      UI32_T uint_ipMask=0;
      UI8_T   ip_address[SYS_ADPT_IPV4_ADDR_LEN];
      UI8_T   mask[SYS_ADPT_IPV4_ADDR_LEN];
      UI8_T   bcast_addr[SYS_ADPT_IPV4_ADDR_LEN];
      UI8_T   network_id[SYS_ADPT_IPV4_ADDR_LEN];
   
 switch(cmd_idx)
 {
  case PRIVILEGE_CFG_INTERFACE_ETH0_CMD_W2_IP_ADDRESS:
  CLI_LIB_AtoIp(arg[0], (UI8_T*)&uint_ipAddress);
  CLI_LIB_AtoIp(arg[1], (UI8_T*)&uint_ipMask);

  ip_address[0] = (uint_ipAddress&0xff000000)>>24;
  ip_address[1] = (uint_ipAddress&0xff0000)>>16;
  ip_address[2] = (uint_ipAddress&0xff00)>>8;
  ip_address[3] = (uint_ipAddress&0xff);
  mask[0] = (uint_ipMask&0xff000000)>>24;
  mask[1] = (uint_ipMask&0xff0000)>>16;
  mask[2] = (uint_ipMask&0xff00)>>8;
  mask[3] = (uint_ipMask&0xff);
  
             /* check ip_address, ip_mask,
             * 1. must be either class A, class B, or class C
             * 2. can't be network b'cast ip or network id
             * 3. can't be loop back ip
             * 4. can't be b'cast ip
             * 5. can't be m'cast ip
             */
            
             /* class A, class B or class C */
             if((IP_LIB_IsIpInClassA(ip_address) == FALSE) && (IP_LIB_IsIpInClassB(ip_address) == FALSE) &&
                     (IP_LIB_IsIpInClassC(ip_address) == FALSE) &&
                         (((ip_address[0] & mask[0])!= 0) ||
                         ((ip_address[1] & mask[1])!= 0) ||
                         ((ip_address[2] & mask[2])!= 0) ||
                         ((ip_address[3] & mask[3])!= 0)))
             {
                 CLI_LIB_PrintStr("Invalid IP address. Must be either class A, class B, or class C.\r\n");
                 return CLI_NO_ERROR;
             }
            
             /*  network b'cast ip   */
             if (IP_LIB_GetSubnetBroadcastIp(ip_address, mask, bcast_addr) == IP_LIB_OK)
             {
                 if(memcmp(bcast_addr, ip_address, SYS_ADPT_IPV4_ADDR_LEN) == 0)
                 {
                     CLI_LIB_PrintStr("Invalid IP address. Can't be network b'cast IP.\r\n");
                     return CLI_NO_ERROR;
                 }
             }
            
              /*  network ID  */
              IP_LIB_GetNetworkID(ip_address, mask, network_id);
              if (memcmp(network_id, ip_address, SYS_ADPT_IPV4_ADDR_LEN) == 0 )
              {
                  CLI_LIB_PrintStr("Invalid IP address. Can't be network ID.\r\n");
                  return CLI_NO_ERROR;
              }
            
             /* loopback ip, b'cast ip, or m'cast ip */
             if ((IP_LIB_IsLoopBackIp (ip_address) == TRUE) ||
                 (IP_LIB_IsMulticastIp(ip_address) == TRUE) ||
                 (IP_LIB_IsBroadcastIp(ip_address) == TRUE))
             {
                 CLI_LIB_PrintStr("Invalid IP address. Can't be loopback IP, b'cast IP, or m'cast IP.\r\n");
                 return CLI_NO_ERROR;
             }

  /*set ethernet0 ip address*/
  if(setEth0IpAddr("eth",0,uint_ipAddress,uint_ipMask)==-1)
             {
                  CLI_LIB_PrintStr("Failed to add IP address of Ethernet0.\r\n");
             }
  break;
            
  case PRIVILEGE_CFG_INTERFACE_ETH0_CMD_W3_NO_IP_ADDRESS:
              if(delEth0IpAddr("eth",0)==-1)
              {
                  CLI_LIB_PrintStr("Failed to delete IP address of Ethernet0.\r\n");
              }
         break;
            
   default:
    return CLI_ERR_INTERNAL;          
 }

  return CLI_NO_ERROR;

}
#endif
/*fuzhimin,20090106,end*/


/*change mode*/

/*execution*/

/* cmd: show ip interface */
#if (CLI_SUPPORT_L3_FEATURE != 1)
UI32_T CLI_API_Show_Ip_Interfaces(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MULTIPLE_MGMT_IP == TRUE)
    UI32_T rif_vid = 0, target_id = 0;
#endif
    UI32_T line_num = 0;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    NETCFG_TYPE_L3_Interface_T ip_interface;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN];
#if (SYS_CPNT_MULTIPLE_MGMT_IP != TRUE)
    UI32_T manage_vid = 0;
#endif

    memset(&ip_interface, 0, sizeof(NETCFG_TYPE_L3_Interface_T));

#if (SYS_CPNT_MULTIPLE_MGMT_IP != TRUE)
    if(TRUE == VLAN_POM_GetManagementVlan(&manage_vid))
    {
        VLAN_VID_CONVERTTO_IFINDEX(manage_vid, ip_interface.ifindex);
        if(NETCFG_TYPE_OK == NETCFG_POM_IP_GetL3Interface(&ip_interface))
        {
                SYSFUN_Sprintf(buff, "VLAN %lu is ", (unsigned long)manage_vid);

	        if (IP_LIB_IsIpInterfaceUp(ip_interface.u.physical_intf.if_flags) == TRUE)
	            SYSFUN_Strncat(buff,"Administrative Up",100);
	        else
	            SYSFUN_Strncat(buff,"Administrative Down",100);
	
	        if (IP_LIB_IsIpInterfaceRunning(ip_interface.u.physical_intf.if_flags) == TRUE)
	            SYSFUN_Strncat(buff," - Link Up\r\n",100);
	        else
	            SYSFUN_Strncat(buff," - Link Down\r\n",100);
	        PROCESS_MORE(buff);
	
	        SYSFUN_Sprintf(buff, "  Address is %02X-%02X-%02X-%02X-%02X-%02X\r\n", ip_interface.u.physical_intf.logical_mac[0], ip_interface.u.physical_intf.logical_mac[1], 
	                        ip_interface.u.physical_intf.logical_mac[2], ip_interface.u.physical_intf.logical_mac[3], ip_interface.u.physical_intf.logical_mac[4], 
	                        ip_interface.u.physical_intf.logical_mac[5]);
	        PROCESS_MORE(buff);
	
	        SYSFUN_Sprintf(buff,"  Index: %lu, MTU: %lu\r\n",
	                        (unsigned long)ip_interface.ifindex, (unsigned long)ip_interface.u.physical_intf.mtu);
	        PROCESS_MORE(buff);
	
	        switch(ip_interface.u.physical_intf.ipv4_address_mode)
	        {
	            case NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE:
	                SYSFUN_Sprintf(buff,"  Address Mode is User specified\r\n");
	                break;
	
	#if (SYS_CPNT_DHCP_CLIENT == TRUE)
	            case NETCFG_TYPE_IP_ADDRESS_MODE_DHCP:
	                SYSFUN_Sprintf(buff,"  Address Mode is DHCP\r\n");
	                break;       
	#endif         
    #if (SYS_CPNT_BOOTP == TRUE)  
	            case NETCFG_TYPE_IP_ADDRESS_MODE_BOOTP:
	                SYSFUN_Sprintf(buff,"  Address Mode is BOOTP\r\n");
	                break;           
    #endif
	            default:
	                SYSFUN_Sprintf(buff,"  Address Mode is not specified\r\n");
	                break;  
	        }
	        PROCESS_MORE(buff);
	
	        memset(&rif_config, 0, sizeof(rif_config));
	        /* To get first one entry, set ip_addr to zero array */
	        memset(rif_config.addr.addr, 0, SYS_ADPT_IPV4_ADDR_LEN);
	        rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;
	        rif_config.ifindex = ip_interface.ifindex;
	
	        while (NETCFG_POM_IP_GetNextInetRifOfInterface(&rif_config) == NETCFG_TYPE_OK)
	        {
	            if((rif_config.addr.type != L_INET_ADDR_TYPE_IPV4) 
	                && (rif_config.addr.type != L_INET_ADDR_TYPE_IPV4Z))
	                break;
	            IP_LIB_CidrToMask(rif_config.addr.preflen,  mask);
	            SYSFUN_Sprintf(buff,"  IP Address: %d.%d.%d.%d Mask: %d.%d.%d.%d\r\n", 
	                            rif_config.addr.addr[0], rif_config.addr.addr[1], 
	                            rif_config.addr.addr[2], rif_config.addr.addr[3],
	                            mask[0], mask[1], mask[2], mask[3]);
	            PROCESS_MORE(buff);
	        }
        }
    }
#else /* #if (SYS_CPNT_MULTIPLE_MGMT_IP != TRUE) */
    while(NETCFG_POM_IP_GetNextL3Interface(&ip_interface) == NETCFG_TYPE_OK)
    {
        /* vlan is specified */
        if(arg[0] && (arg[0][0]=='v')) /* vlan */
        {
            target_id = atoi(arg[1]);
            if(FALSE == VLAN_OM_ConvertFromIfindex(ip_interface.ifindex, &rif_vid))
                continue;

            if(rif_vid != target_id)
                continue;
        }
        else if(arg[0] && (arg[0][0]=='l')) /* loopback int. is specified */
        {
            if (FALSE == IP_LIB_IsLoopbackInterface(ip_interface.u.physical_intf.if_flags))
                continue;

        }

        if (TRUE == IP_LIB_IsLoopbackInterface(ip_interface.u.physical_intf.if_flags))
        {
#if (SYS_CPNT_LOOPBACK_IF_VISIBLE == TRUE)
            IP_LIB_ConvertLoopbackIfindexToId(ip_interface.ifindex, &rif_vid);
            SYSFUN_Sprintf(buff,"Loopback %lu is ", (unsigned long)rif_vid);
#else
            /* skip loopback interface */
            continue;
#endif
        }
        else
        {
            VLAN_OM_ConvertFromIfindex(ip_interface.ifindex, &rif_vid);
            SYSFUN_Sprintf(buff,"VLAN %lu is ", (unsigned long)rif_vid);
        }
        if (IP_LIB_IsIpInterfaceUp(ip_interface.u.physical_intf.if_flags) == TRUE)
            SYSFUN_Strncat(buff,"Administrative Up",100);
        else
            SYSFUN_Strncat(buff,"Administrative Down",100);
	
        if (IP_LIB_IsIpInterfaceRunning(ip_interface.u.physical_intf.if_flags) == TRUE)
            SYSFUN_Strncat(buff," - Link Up\r\n",100);
	else
            SYSFUN_Strncat(buff," - Link Down\r\n",100);
	PROCESS_MORE(buff);

        SYSFUN_Sprintf(buff,"  Address is %02X-%02X-%02X-%02X-%02X-%02X\r\n", ip_interface.u.physical_intf.logical_mac[0], ip_interface.u.physical_intf.logical_mac[1],
                        ip_interface.u.physical_intf.logical_mac[2], ip_interface.u.physical_intf.logical_mac[3], ip_interface.u.physical_intf.logical_mac[4],
                        ip_interface.u.physical_intf.logical_mac[5]);
        PROCESS_MORE(buff);

        SYSFUN_Sprintf(buff,"  Index: %lu, MTU: %lu\r\n",
                        (unsigned long)ip_interface.ifindex, (unsigned long)ip_interface.u.physical_intf.mtu);
        PROCESS_MORE(buff);

        switch(ip_interface.u.physical_intf.ipv4_address_mode)
        {
            case NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE:
                SYSFUN_Sprintf(buff,"  Address Mode is User specified\r\n");
                break;

#if (SYS_CPNT_DHCP_CLIENT == TRUE)
            case NETCFG_TYPE_IP_ADDRESS_MODE_DHCP:
                SYSFUN_Sprintf(buff,"  Address Mode is DHCP\r\n");
                break;
#endif
            case NETCFG_TYPE_IP_ADDRESS_MODE_BOOTP:
                SYSFUN_Sprintf(buff,"  Address Mode is BOOTP\r\n");
                break;

            default:
                SYSFUN_Sprintf(buff,"  Address Mode is not specified\r\n");
                break;
        }
        PROCESS_MORE(buff);

        memset(&rif_config, 0, sizeof(rif_config));
         /* get primary rif first */
        rif_config.ifindex = ip_interface.ifindex;
        if(NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config) == NETCFG_TYPE_OK)
        {
            if(rif_config.row_status != NETCFG_TYPE_StaticIpCidrEntryRowStatus_active)
                continue;

            if((rif_config.addr.type != L_INET_ADDR_TYPE_IPV4)
                && (rif_config.addr.type != L_INET_ADDR_TYPE_IPV4Z))
                break;

            IP_LIB_CidrToMask(rif_config.addr.preflen, mask);
            SYSFUN_Sprintf(buff,"  IP Address: %d.%d.%d.%d Mask: %d.%d.%d.%d\r\n",
                            rif_config.addr.addr[0], rif_config.addr.addr[1],
                            rif_config.addr.addr[2], rif_config.addr.addr[3],
                            mask[0], mask[1], mask[2], mask[3]);
            PROCESS_MORE(buff);
    }

        /* get secondary rif */
        memset(&rif_config, 0, sizeof(rif_config));
        rif_config.ifindex = ip_interface.ifindex;
        while(NETCFG_POM_IP_GetNextSecondaryRifFromInterface(&rif_config) == NETCFG_TYPE_OK)
        {
            if(rif_config.row_status != NETCFG_TYPE_StaticIpCidrEntryRowStatus_active)
                continue;

            if((rif_config.addr.type != L_INET_ADDR_TYPE_IPV4)
                && (rif_config.addr.type != L_INET_ADDR_TYPE_IPV4Z))
                break;

            IP_LIB_CidrToMask(rif_config.addr.preflen, mask);
            SYSFUN_Sprintf(buff,"  IP Address: %d.%d.%d.%d Mask: %d.%d.%d.%d Secondary\r\n",
                            rif_config.addr.addr[0], rif_config.addr.addr[1],
                            rif_config.addr.addr[2], rif_config.addr.addr[3],
                            mask[0], mask[1], mask[2], mask[3]);
            PROCESS_MORE(buff);
        }
    } /* while */
#endif /* #if (SYS_CPNT_MULTIPLE_MGMT_IP != TRUE) */

   
/*fuzhimin,20090106*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
    /*show ethernet0*/
    {
        struct ifreq ifr;
        int ret;
        int  ipaddr;
        int  netmask;
        

        bzero ((caddr_t)&ifr, sizeof (ifr));
        strcpy (ifr.ifr_name, "eth0");
        ret = if_ioctl (SIOCGIFFLAGS, (caddr_t) &ifr);
        if (ret < 0)
        {
            /*printf(" Failed SIOCGIFFLAGS for eth0\n");*/
            return (CLI_NO_ERROR);
        }
        if(ifr.ifr_flags & IFF_UP)
        {
            CLI_LIB_PrintStr((char *)"Ethernet0 is Administrative Up\r\n");
            ret = if_ioctl (SIOCGIFHWADDR, (caddr_t) &ifr);
            if (ret < 0)
            {
                /*printf(" Failed SIOCGIFHWADDR for eth0\n");*/
                return (CLI_NO_ERROR);
            }
    
            memset(buff,0,CLI_DEF_MAX_BUFSIZE);
            SYSFUN_Sprintf((char *)buff, "  Address is %02X-%02X-%02X-%02X-%02X-%02X\r\n", ifr.ifr_hwaddr.sa_data[0], ifr.ifr_hwaddr.sa_data[1], 
                            ifr.ifr_hwaddr.sa_data[2], ifr.ifr_hwaddr.sa_data[3], ifr.ifr_hwaddr.sa_data[4], 
                            ifr.ifr_hwaddr.sa_data[5]);
    
            CLI_LIB_PrintStr(buff);
    
            ret = if_ioctl (SIOCGIFMTU, (caddr_t) &ifr);
            if (ret < 0)
            {
                /*printf(" Failed SIOCGIFMTU for eth0\n");*/
                return (CLI_NO_ERROR);
            }
    
            CLI_LIB_PrintStr_1("  MTU: %d\r\n",ifr.ifr_mtu);
    
            ret = if_ioctl (SIOCGIFADDR, (caddr_t) &ifr);
            if (ret < 0)
            {
                /*printf(" Failed SIOCGIFADDR for eth0\n");*/
                return (CLI_NO_ERROR);
            }
            ipaddr = ((struct sockaddr_in*)(&(ifr.ifr_addr)))->sin_addr.s_addr;
            CLI_LIB_PrintStr_4("  IP Address: %d.%d.%d.%d", 
                                (ipaddr&0xff000000)>>24, (ipaddr&0xff0000)>>16, 
                                (ipaddr&0xff00)>>8, (ipaddr&0xff));
    
            ret = if_ioctl (SIOCGIFNETMASK, (caddr_t) &ifr);
            if (ret < 0)
            {
                /*printf(" Failed SIOCGIFNETMASK for eth0\n");*/
                return (CLI_NO_ERROR);
            }
            netmask = ((struct sockaddr_in*)(&(ifr.ifr_netmask)))->sin_addr.s_addr;
            CLI_LIB_PrintStr_4(" Mask: %d.%d.%d.%d", 
                                (netmask&0xff000000)>>24, (netmask&0xff0000)>>16, 
                                (netmask&0xff00)>>8, (netmask&0xff));
            CLI_LIB_PrintStr((char *)"\r\n");
        }
#if 0
{
 struct phy_device *phydev;
 phydev = get_fcc_phy_device();
    
  CLI_LIB_PrintStr_2("PHY: %s - Link is %s", phydev->dev.bus_id,
   phydev->link ? "Up" : "Down");
 if (phydev->link)
  CLI_LIB_PrintStr_2(" - %d/%s", phydev->speed,
    0x0/*DUPLEX_FULL*/ == phydev->duplex ?
    "Full" : "Half");

 CLI_LIB_PrintStr("(UI8_T *)\n");
        
}
#endif  
    }
#endif 
#if (SYS_CPNT_CRAFT_PORT == TRUE)
    /*show craft interface */
    {
        NETCFG_TYPE_CraftInetAddress_T craft_addr;
        memset(&craft_addr, 0, sizeof(craft_addr));
        if(NETCFG_POM_IP_GetCraftInterfaceInetAddress(&craft_addr) == NETCFG_TYPE_OK)
        {
            CLI_LIB_PrintStr("Craft interface is Administrative Up\r\n");
            CLI_LIB_PrintStr_4("  IP Address: %d.%d.%d.%d",
                            craft_addr.addr.addr[0], craft_addr.addr.addr[1],
                            craft_addr.addr.addr[2], craft_addr.addr.addr[3]);
            IP_LIB_CidrToMask(craft_addr.addr.preflen, mask);
            CLI_LIB_PrintStr_4(" Mask: %d.%d.%d.%d\r\n", mask[0], mask[1], mask[2], mask[3]);
            PROCESS_MORE(buff);
             
        }
    }
#endif /* SYS_CPNT_CRAFT_PORT */

    return CLI_NO_ERROR; 
}

/* cmd: show ip default-gateway */
UI32_T CLI_API_Show_Ip_Redirects(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   L_INET_AddrIp_T default_gateway;
   default_gateway.type = L_INET_ADDR_TYPE_IPV4;

    if(NETCFG_TYPE_OK == NETCFG_PMGR_ROUTE_GetDefaultGateway(&default_gateway))
        CLI_LIB_PrintStr_4("IP default gateway %d.%d.%d.%d\r\n", default_gateway.addr[0], default_gateway.addr[1], default_gateway.addr[2], default_gateway.addr[3]);
    return CLI_NO_ERROR; 
}

/* command: show ip traffic */
#if 0 /* output example */
IPv4 Statistics:
IPv4 recived
                3886 rcvd total total received
                   0 header errors
IPv4 sent
                   0 forwarded datagrams
ICMPv4 Statistics:
ICMPv4 received
                3885 input
ICMPv4 sent
                  12 output
UDP Statistics:
                   1 input

#endif

#if 0 // move to cli_api_system.c 
UI32_T CLI_API_Show_Ip_Traffic(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    struct ip_mib ipstat;
    struct icmp_mib icmpstat;
    struct udp_mib udpstat;
    struct tcp_mib tcpstat;

    memset(&ipstat, 0, sizeof(ipstat));

    /* get IPv4 statistics */
    if(IPAL_RESULT_OK  != IPAL_IF_GetAllIpv4Statistic(&ipstat))
    {
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr("IP Statistics:\n");

    /* IPv4 received statistic */
    CLI_LIB_PrintStr("IP received\n");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipInReceives, "total received");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipInHdrErrors, "header errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipInUnknownProtos, "unknown protocols");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipInAddrErrors, "address errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipInDiscards, "discards");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipInDelivers, "delivers");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipReasmReqds, "reassembly request datarams");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipReasmOKs, "reassembled succeeded");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipReasmFails, "reassembled failed");

    /* IPv4 sent statistic */

    CLI_LIB_PrintStr("IP sent\n");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipForwDatagrams, "forwards datagrams");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipOutRequests, "requests");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipOutDiscards, "discards");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipOutNoRoutes, "no routes");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipFragCreates, "generated fragments");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipFragOKs, "fragment succeeded");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", ipstat.ipFragFails, "fragment failed");


    /* get ICMP statistics */
    if(IPAL_RESULT_OK  != IPAL_IF_GetAllIcmpStatistic(&icmpstat))
    {
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr("ICMP Statistics:\n");

    /* ICMP received statistics */
    CLI_LIB_PrintStr("ICMP received\n");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInMsgs, "input");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInErrors, "errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInDestUnreachs, "destination unreachable messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInTimeExcds, "time exceeded messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInParmProbs, "parameter problem message");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInEchos, "echo request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInEchoReps, "echo reply messages");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInRedirects, "redirect messages");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInTimestamps, "timestamp request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInTimestampReps, "timestamp reply messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInSrcQuenchs, "source quench messages");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInAddrMasks, "address mask request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpInAddrMaskReps, "address mask reply messages");

    /* ICMP sent statistics */
    CLI_LIB_PrintStr("ICMP sent\n");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutMsgs, "output");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutErrors, "errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutDestUnreachs, "destination unreachable messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutTimeExcds, "time exceeded messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutParmProbs, "parameter problem message");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutEchos, "echo request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutEchoReps, "echo reply messages");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutRedirects, "redirect messages");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutTimestamps, "timestamp request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutTimestampReps, "timestamp reply messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutSrcQuenchs, "source quench messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutAddrMasks, "address mask request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", icmpstat.icmpOutAddrMaskReps, "address mask reply messages");


    /* get UDP statistics */
    if(IPAL_RESULT_OK  != IPAL_IF_GetAllUdpStatistic(&udpstat))
    {
        return CLI_NO_ERROR;
    }

    /* UDP statistics */
    CLI_LIB_PrintStr("UDP Statistics:\n");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", udpstat.udpInDatagrams, "input");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", udpstat.udpNoPorts, "no port errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", udpstat.udpInErrors, "other errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", udpstat.udpOutDatagrams, "output");


    /* get TCP statistics */
    if(IPAL_RESULT_OK  != IPAL_IF_GetAllTcpStatistic(&tcpstat))
    {
        return CLI_NO_ERROR;
    }

    /* TCP statistics */
    CLI_LIB_PrintStr("TCP Statistics:\n");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", tcpstat.tcpInSegs, "input");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", tcpstat.tcpInErrs, "input errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", tcpstat.tcpOutSegs, "output");
   
    return CLI_NO_ERROR; 
}
#endif

/* cmd: ip address */
UI32_T CLI_API_Ip_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T ret;
    UI32_T vid;
    UI32_T uint_ipMask    = 0;
    UI32_T mode = 0;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    UI8_T byte_mask[SYS_ADPT_IPV4_ADDR_LEN] = {};
    L_INET_AddrIp_T default_gateway;
    UI32_T uint_gateway = 0;
    UI8_T byte_gateway[SYS_ADPT_IPV4_ADDR_LEN] = {};
    BOOL_T has_default_gateway = FALSE;
    BOOL_T is_secondary = FALSE;
    int i;
   
/*if support cfgdb ip address can not set when provision*/
#if (SYS_CPNT_CFGDB == TRUE)
#if (SYS_CPNT_NETCFG_IP_ADDRESS_IN_CFGDB == TRUE)
    if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
    {
        return CLI_NO_ERROR;	
    }
#endif
#endif


    memset(&rif_config, 0, sizeof(rif_config));
    VLAN_OM_ConvertFromIfindex(ctrl_P->CMenu.vlan_ifindex, &vid);
   
    switch (cmd_idx)                                                      
    {                                               
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_IP_ADDRESS: 
        
            if(arg[0]==NULL)
                return CLI_ERR_INTERNAL;
                
            switch(*arg[0])
            {
#if (SYS_CPNT_DHCP_CLIENT == TRUE)  	
                case 'd':
                case 'D':
                    if(NETCFG_PMGR_IP_SetIpAddressMode(ctrl_P->CMenu.vlan_ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_DHCP) != NETCFG_TYPE_OK)
                    {
#if 0
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as DHCP mode on VLAN %lu\r\n", (unsigned long)vid);
#endif
#endif
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as DHCP mode on VLAN %lu\r\n", (unsigned long)vid);
                    }
                    else
                    {
#if 0 /* rich  */
                        DHCP_PMGR_Restart();	
#endif
            
                    }
                    break;
#endif          
#if (SYS_CPNT_BOOTP == TRUE)  
                case 'b':
                case 'B':
                    if(NETCFG_PMGR_IP_SetIpAddressMode (ctrl_P->CMenu.vlan_ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_BOOTP) != NETCFG_TYPE_OK)
                    {
#if 0
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as BOOTP mode on VLAN %lu\r\n", (unsigned long)vid);
#endif
#endif
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as BOOTP mode on VLAN %lu\r\n", (unsigned long)vid);
                    }
                    break;
#endif
                default:				
                    memset(&rif_config, 0, sizeof(rif_config));
                    rif_config.ifindex = ctrl_P->CMenu.vlan_ifindex;
                    for (i = 1; arg[i];)
                    {
                        if (strncmp(arg[i], "secondary", 3) == 0)
                        {
                            is_secondary = TRUE;
                            i++;
                        }
                        else
                            i++;
                    }
                    if(is_secondary)
                        rif_config.ipv4_role = NETCFG_TYPE_MODE_SECONDARY; 
                    else
                        rif_config.ipv4_role = NETCFG_TYPE_MODE_PRIMARY; /* default */
 
                    rif_config.row_status = VAL_netConfigStatus_2_createAndGo;
                    if((arg[1]!=NULL) && CLI_LIB_AtoIp(arg[1], (UI8_T*)&uint_ipMask))
                    {
                        /* format: 192.168.1.1 255.255.255.0 */
                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                                           arg[0],
                                                                           (L_INET_Addr_T *) &rif_config.addr,
                                                                           sizeof(rif_config.addr)))
                        {
                            CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                            return CLI_NO_ERROR;
                        }
                        
                        /* convert to byte array format */
                        IP_LIB_UI32toArray(uint_ipMask, byte_mask);

                        if(!IP_LIB_IsValidNetworkMask(byte_mask))
                        {
                            CLI_LIB_PrintStr_1("Invalid Netmask address - %s\r\n", arg[1]);
                            return CLI_NO_ERROR;
                        }
                        rif_config.addr.preflen = IP_LIB_MaskToCidr(byte_mask);
                   }
                    else
                    {   
                        /* format: 192.168.1.1/24 */
                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                                           arg[0],
                                                                           (L_INET_Addr_T *) &rif_config.addr,
                                                                           sizeof(rif_config.addr)))
                        {
                            CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                            return CLI_NO_ERROR;
                        }

                        IP_LIB_CidrToMask(rif_config.addr.preflen, byte_mask);
 
                    }

                    ret = IP_LIB_IsValidForIpConfig(rif_config.addr.addr, byte_mask);
                    switch(ret)
                    {
                        case IP_LIB_INVALID_IP:
                        case IP_LIB_INVALID_IP_LOOPBACK_IP:
                        case IP_LIB_INVALID_IP_ZERO_NETWORK:
                        case IP_LIB_INVALID_IP_BROADCAST_IP:
                        case IP_LIB_INVALID_IP_IN_CLASS_D:
                        case IP_LIB_INVALID_IP_IN_CLASS_E:
                            CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                            return CLI_NO_ERROR;
                            break;
                       case IP_LIB_INVALID_IP_SUBNET_NETWORK_ID:
                            CLI_LIB_PrintStr("Invalid IP address. Can't be network ID.\r\n");
                            return CLI_NO_ERROR;
                            break;
                        case IP_LIB_INVALID_IP_SUBNET_BROADCAST_ADDR:

                            CLI_LIB_PrintStr("Invalid IP address. Can't be network b'cast IP.\r\n");
                            return CLI_NO_ERROR;

                        default:
                            break;
                    }

                    /* Check invalid default-gateway (optional) */
                    for(i=1;arg[i];)
                    {
                        if (strncmp(arg[i], "default-gateway", 3) == 0)
                        {
                            if(arg[i+1]==NULL)
                            {                        
                                CLI_LIB_PrintStr("Need default-gateway.\r\n");
                                return CLI_NO_ERROR;
                            }
                            if(!CLI_LIB_AtoIp(arg[i+1], (UI8_T*)&uint_gateway))
                            {
                                CLI_LIB_PrintStr_1("Invalid default-gateway - %s\r\n", arg[i+1]);
                                return CLI_NO_ERROR;
                            }
                            if(uint_gateway == 0)
                            {
                                CLI_LIB_PrintStr_1("Invalid default-gateway - %s\r\n", arg[i+1]);
                                return CLI_NO_ERROR;
                            }                    
                            IP_LIB_UI32toArray(uint_gateway, byte_gateway);
                            if(!IP_LIB_IsIpBelongToSubnet(rif_config.addr.addr, rif_config.addr.preflen, byte_gateway))
                            {
                                CLI_LIB_PrintStr("Invalid default-gateway. Should be in the same subnet with IP address.\r\n");
                                return CLI_NO_ERROR;
                            }
                            memset(&default_gateway, 0, sizeof(default_gateway));

                            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                                               arg[i+1],
                                                                               (L_INET_Addr_T *) &default_gateway,
                                                                               sizeof(default_gateway)))
                            {
                                CLI_LIB_Printf("Invalid default-gateway - %s\r\n", arg[i+1]);
                                break;
                            }

                              
                            has_default_gateway = TRUE;
                            i+=2;
                        }
                        else
                            i++;
                    } /* for */

                    if( NETCFG_PMGR_IP_SetIpAddressMode (ctrl_P->CMenu.vlan_ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE) != NETCFG_TYPE_OK)
                    {
#if 0
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as user define mode on VLAN %lu\r\n", (unsigned long)vid);
#endif
#endif
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as user define mode on VLAN %lu\r\n", (unsigned long)vid);
                    }
                    else
                    {
                        ret = NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
                        if(ret != NETCFG_TYPE_OK)
                        {
#if 0
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set IP address on VLAN %lu\r\n", (unsigned long)vid); 
#endif
#endif
                            CLI_LIB_PrintStr_1("Failed to set IP address on VLAN %lu\r\n", (unsigned long)vid); 
                        }
                        else
                        {
                            /* default-gateway (optional) */
                            if(has_default_gateway)
                            {
                                if(NETCFG_PMGR_ROUTE_AddDefaultGateway(&default_gateway, SYS_DFLT_DEFAULT_GATEWAY_METRIC)!=NETCFG_TYPE_OK)
                                    CLI_LIB_PrintStr("Failed to set default gateway\r\n");
                            }
                        }
                    }
                    break;
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_NO_IP_ADDRESS: 
            if(arg[0] == NULL)    /* no ip address */
            {
                memset(&rif_config, 0, sizeof(rif_config));
                rif_config.ifindex = ctrl_P->CMenu.vlan_ifindex;
                /* we must destroy secondary rif first */
                while(NETCFG_TYPE_OK == NETCFG_POM_IP_GetNextSecondaryRifFromInterface(&rif_config))
                { 
                    rif_config.row_status = VAL_netConfigStatus_2_destroy;
       
                    if(NETCFG_TYPE_OK != NETCFG_PMGR_IP_SetInetRif(&rif_config,NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB))
                    {                 
                        CLI_LIB_PrintStr_1("Failed to destory IP address on VLAN %lu\r\n", (unsigned long)vid);
                    }
                }

                /* Destroy primary rif at last */
                memset(&rif_config, 0, sizeof(rif_config));
                rif_config.ifindex = ctrl_P->CMenu.vlan_ifindex;
                if(NETCFG_TYPE_OK == NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config))
                {
                    rif_config.row_status = VAL_netConfigStatus_2_destroy;
                    if(NETCFG_TYPE_OK != NETCFG_PMGR_IP_SetInetRif(&rif_config,NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB))
                    {
                        CLI_LIB_PrintStr_1("Failed to destory IP address on VLAN %lu\r\n", (unsigned long)vid);
                    }
                }
            }
            else
            {
                if((arg[0][0] == 'D') || (arg[0][0] == 'd'))  /* no ip address dhcp */
                {                 

                    /* if command is "no ip address dhcp, need to clear client lease and send DHCPREALEASE additionally "*/
                    if((ret = DHCP_PMGR_ReleaseClientLease(ctrl_P->CMenu.vlan_ifindex))!=DHCP_MGR_OK)
                    {
                         CLI_LIB_PrintStr("Failed to release dhcp client lease\r\n");
                    }
                    
                    if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetIpAddressMode(ctrl_P->CMenu.vlan_ifindex,&mode))
                    {   
                        CLI_LIB_PrintStr_1("Failed to get the IP address mode on VLAN %lu\r\n", (unsigned long)vid);
                        return CLI_NO_ERROR;
                    }

                    if(mode != NETCFG_TYPE_IP_ADDRESS_MODE_DHCP)
                    {
                        CLI_LIB_PrintStr("Current IP address mode is not DHCP\r\n");
                        return CLI_NO_ERROR;
                    }

                    if( NETCFG_TYPE_OK != NETCFG_PMGR_IP_SetIpAddressMode (ctrl_P->CMenu.vlan_ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE))
                    {
                         CLI_LIB_PrintStr_1("Failed to set the IP address mode as user defined mode on VLAN %lu\r\n", (unsigned long)vid)
                    }
                  
                    
                }
                else
                {			
                    /* get vlan ip first*/     
                    memset(&rif_config, 0, sizeof(rif_config));
                    rif_config.ifindex = ctrl_P->CMenu.vlan_ifindex;
                    for (i = 1; arg[i];)
                    {
                        if (strncmp(arg[i], "secondary", 3) == 0)
                        {
                            is_secondary = TRUE;
                            i++;
                        }
                        else
                            i++;
                    }
                     if(is_secondary)
                        rif_config.ipv4_role = NETCFG_TYPE_MODE_SECONDARY; 
                    else
                        rif_config.ipv4_role = NETCFG_TYPE_MODE_PRIMARY; /* default */
                     if((arg[1]!=NULL) && CLI_LIB_AtoIp(arg[1], (UI8_T*)&uint_ipMask))
                    {
                        /* format: 192.168.1.1 255.255.255.0 */
                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                                           arg[0],
                                                                           (L_INET_Addr_T *) &rif_config.addr,
                                                                           sizeof(rif_config.addr)))
                        {
                            CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                            return CLI_NO_ERROR;
                        }
                    
                        IP_LIB_UI32toArray(uint_ipMask, byte_mask);
                        rif_config.addr.preflen = IP_LIB_MaskToCidr(byte_mask);
                    }
                    else
                    {
                        /* format: 192.168.1.1/24 */
                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                                           arg[0],
                                                                           (L_INET_Addr_T *) &rif_config.addr,
                                                                           sizeof(rif_config.addr)))
                        {
                            CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                            return CLI_NO_ERROR;
                        }
                    }

                    if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetRifFromInterface(&rif_config)) 
                    {     
                        /* There is no matched address on this VLAN */
                        CLI_LIB_PrintStr_1("Can't find address on VLAN %lu\r\n", (unsigned long)vid);
                        break;
                    }
                    rif_config.row_status = VAL_netConfigStatus_2_destroy;
               
                    if( NETCFG_PMGR_IP_SetIpAddressMode (ctrl_P->CMenu.vlan_ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE) != NETCFG_TYPE_OK)
                    {
        #if 0
        #if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
        #else
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as user define mode on VLAN %lu\r\n", (unsigned long)vid)
        #endif
        #endif
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as user define mode on VLAN %lu\r\n", (unsigned long)vid)
                    }
                    else
                    {
                        ret = NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
                            
                        if(ret != NETCFG_TYPE_OK)
                        {
        #if 0
        #if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
        #else
                            CLI_LIB_PrintStr_1("Failed to remove IP address on VLAN %lu\r\n", (unsigned long)vid);
        #endif
        #endif
                            CLI_LIB_PrintStr_1("Failed to remove IP address on VLAN %lu\r\n", (unsigned long)vid);
                        }
                    }
                }
            }
            break;

        default:     
            break;
    }
    return CLI_NO_ERROR;
}

UI32_T CLI_API_IP_Dhcp_Restart(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_CLIENT == TRUE)
    DHCP_PMGR_Restart();	
#endif
    return CLI_NO_ERROR; 
}

UI32_T CLI_API_Ip_Defaultgateway(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    L_INET_AddrIp_T default_gateway;
/*if support cfgdb ip address can not set when provision*/
#if (SYS_CPNT_CFGDB == TRUE)
#if (SYS_CPNT_NETCFG_IP_ADDRESS_IN_CFGDB == TRUE)
    if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
    {
        return CLI_NO_ERROR;	
    }
#endif
#endif
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_IP_DEFAULTGATEWAY:
            memset(&default_gateway, 0, sizeof(default_gateway));

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                               arg[0],
                                                               (L_INET_Addr_T *) &default_gateway,
                                                               sizeof(default_gateway)))
            {
                CLI_LIB_Printf("Invalid nexthop address.\r\n");
                break;
            }

            if(IP_LIB_IsLoopBackIp(default_gateway.addr) == TRUE
                || IP_LIB_IsZeroNetwork(default_gateway.addr) == TRUE
                || IP_LIB_IsBroadcastIp(default_gateway.addr) == TRUE
                || IP_LIB_IsMulticastIp(default_gateway.addr) == TRUE
                || IP_LIB_IsTestingIp(default_gateway.addr) == TRUE)
            {
                CLI_LIB_Printf("Invalid nexthop address.\r\n");
                return CLI_NO_ERROR;
            }
            
            
            if(NETCFG_PMGR_ROUTE_AddDefaultGateway(&default_gateway, SYS_DFLT_DEFAULT_GATEWAY_METRIC)!=NETCFG_TYPE_OK)
            {
                CLI_LIB_PrintStr("Failed to set default gateway\r\n");
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IP_DEFAULTGATEWAY:
            memset(&default_gateway, 0, sizeof(default_gateway));
    	    default_gateway.type = L_INET_ADDR_TYPE_IPV4;
    	    default_gateway.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            NETCFG_PMGR_ROUTE_GetDefaultGateway(&default_gateway);
	        if(NETCFG_PMGR_ROUTE_DeleteDefaultGateway(&default_gateway)!=NETCFG_TYPE_OK)
            {
                CLI_LIB_PrintStr("Failed to disable the default gateway.\r\n");
            }
            break;

        default:
            return CLI_NO_ERROR;
    }
    return CLI_NO_ERROR;
}

#if 0
UI32_T CLI_API_L3_Ip_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI8_T dst_ipAddress[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI8_T netmask_Address[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI8_T gateway_Address[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI32_T   distance        = SYS_ADPT_MIN_ROUTE_DISTANCE;
    UI8_T    choice[2] = {0};
    UI32_T   return_type = 0;
    BOOL_T all_network = TRUE;

   switch(cmd_idx)
   {
       case PRIVILEGE_CFG_GLOBAL_CMD_W2_IP_ROUTE:

          CLI_LIB_AtoIp(arg[0], dst_ipAddress);
          CLI_LIB_AtoIp(arg[1], netmask_Address);
          CLI_LIB_AtoIp(arg[2], gateway_Address);
 
          if (arg[3] != NULL)
          {
             distance = CLI_LIB_AtoUl(arg[3], 10);
          }
          if(NETCFG_PMGR_ROUTE_AddStaticRoute(dst_ipAddress,netmask_Address,gateway_Address,distance)!=NETCFG_TYPE_OK)
          {
             CLI_LIB_PrintStr("Failed to configure the net-routing operation.\r\n");
             return CLI_NO_ERROR;
          }

          break;

       case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IP_ROUTE:

          if(arg[0][0] == '*')
          {
             CLI_LIB_PrintStr("This operation will delete all static entries in routing table.\r\n");
             CLI_LIB_PrintStr("Are you sure to continue this operation (y/n)?");
             CLI_PARS_ReadLine( choice, sizeof(choice), TRUE, FALSE);
             CLI_LIB_PrintNullStr(1);

             if( choice[0] == 0 || (choice[0] != 'y' && choice[0] != 'Y') )
                return CLI_NO_ERROR;

             return_type = NETCFG_PMGR_ROUTE_DeleteAllStaticIpCidrRoutes();

             if ( return_type == NETCFG_TYPE_OK )
             {
                CLI_LIB_PrintStr("The all static entries have been cleared.\r\n");
             }
             else
             {
                CLI_LIB_PrintStr("Failed to clear all static entries.\r\n");
             }
          }
          else
          {

             CLI_LIB_AtoIp(arg[0], dst_ipAddress);
             CLI_LIB_AtoIp(arg[1], netmask_Address);

             if (arg[2] != NULL)
             {
                all_network = FALSE;
                CLI_LIB_AtoIp(arg[2], gateway_Address);
             }
 
             if (arg[3] != NULL)
             {
                distance = CLI_LIB_AtoUl(arg[3], 10);
             }

             if (all_network == FALSE)
             {
                if (NETCFG_PMGR_ROUTE_DeleteStaticRoute(dst_ipAddress,netmask_Address,gateway_Address,distance)!=NETCFG_TYPE_OK)
                {
                    CLI_LIB_PrintStr("Failed to clear the entry.\r\n");
                }
             }
             else if(NETCFG_PMGR_ROUTE_DeleteStaticRouteByNetwork(dst_ipAddress,netmask_Address)!=NETCFG_TYPE_OK)
             {
                CLI_LIB_PrintStr("Failed to clear the entries.\r\n");
                return CLI_NO_ERROR;
             }

          }
          break;
        default:
            return CLI_NO_ERROR;
   }

   return CLI_NO_ERROR;
}


static const UI8_T *CLI_API_IP_RouteTypeStr(NSM_MGR_RouteType_T type)
{
    switch (type)
    {
        case NSM_MGR_ROUTE_TYPE_IPV4_LOCAL:
            return "connected";
        case NSM_MGR_ROUTE_TYPE_IPV4_STATIC:
            return "static";
        case NSM_MGR_ROUTE_TYPE_IPV4_RIP:
            return "rip";
        case NSM_MGR_ROUTE_TYPE_IPV4_OSPF:
            return "ospf";
        case NSM_MGR_ROUTE_TYPE_IPV4_BGP:
            return "bgp";
        case NSM_MGR_ROUTE_TYPE_IPV4_ISIS:
            return "isis";
        case  NSM_MGR_ROUTE_TYPE_IPV4_DYNAMIC,
            return "dynamic";
        case  NSM_MGR_ROUTE_TYPE_IPV6_LOCAL:
            return "connected";
        case  NSM_MGR_ROUTE_TYPE_IPV6_STATIC:
            return "static";
        case  NSM_MGR_ROUTE_TYPE_IPV6_RIP:
            return "rip";
        case  NSM_MGR_ROUTE_TYPE_IPV6_OSPF:
            return "ospf";
        case  NSM_MGR_ROUTE_TYPE_IPV6_BGP:
            return "bgp";
        case  NSM_MGR_ROUTE_TYPE_IPV6_ISIS:
            return "isis";
        case  NSM_MGR_ROUTE_TYPE_IPV6_DYNAMIC:
            return "dynamic";
        default:
            return "unknown";
    }

    return "unknown";
}


static const UI8_T *CLI_API_IP_RouteSubTypeStr(NSM_MGR_RouteSubType_T type)
{
    switch (type)
    {
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_IA:
            return "IA";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_NSSA_1:
            return "N1";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_NSSA_2:
            return "N2";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_EXTERNAL_1:
            return "E1";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_EXTERNAL_2:
            return "E2";
        case NSM_MGR_ROUTE_SUBTYPE_ISIS_L1:
            return "L1";
        case NSM_MGR_ROUTE_SUBTYPE_ISIS_L2:
            return "L2";
        case NSM_MGR_ROUTE_SUBTYPE_ISIS_IA:
            return "ia";
        case NSM_MGR_ROUTE_SUBTYPE_BGP_MPLS:
        default:
            return "  ";
    }

    return "  ";
}


static const UI8_T *CLI_API_IP_RouteTypeChar(NSM_MGR_RouteType_T type)
{
    switch (type)
    {
        case NSM_MGR_ROUTE_TYPE_IPV4_LOCAL:
        case NSM_MGR_ROUTE_TYPE_IPV6_LOCAL:
            return "C";
        case NSM_MGR_ROUTE_TYPE_IPV4_STATIC:
        case NSM_MGR_ROUTE_TYPE_IPV6_STATIC:
            return "S";
        case NSM_MGR_ROUTE_TYPE_IPV4_RIP:
        case NSM_MGR_ROUTE_TYPE_IPV6_RIP
            return "R";
        case NSM_MGR_ROUTE_TYPE_IPV4_OSPF:
        case NSM_MGR_ROUTE_TYPE_IPV6_OSPF:
            return "O";
        case NSM_MGR_ROUTE_TYPE_IPV4_BGP:
        case NSM_MGR_ROUTE_TYPE_IPV6_BGP:
            return "B";
        case NSM_MGR_ROUTE_TYPE_IPV4_ISIS:
        case NSM_MGR_ROUTE_TYPE_IPV6_ISIS:
            return "i";
        default:
            return "?";
    }

    return "?";
}


static void CLI_API_Show_Ip_Route_Summary(void)
{
    NSM_MGR_RouteType_T type;
    UI32_T total = 0;
    UI32_T number;
    UI32_T fib_number = 0;
    UI8_T  multipath_num = 0;
    BOOL_T show_message = TRUE;

    for (type = NSM_MGR_ROUTE_TYPE_IPV4_LOCAL; type < NSM_MGR_ROUTE_TYPE_IPV4_DYNAMIC; type++)
    {
        number = 0;
        NSM_PMGR_GetRouteNumber(type, &number, &fib_number, &multipath_num);
        if (number > 0)
        {
            if (show_message == TRUE)
            {
                CLI_LIB_PrintStr("IP routing table name is Default-IP-Routing-Table(0)\n");
                CLI_LIB_PrintStr_1("IP routing table maximum-paths is %d\n", multipath_num);
                show_message = FALSE;
            }

            CLI_LIB_PrintStr_2("%-15s %ld\n", CLI_API_IP_RouteTypeStr(type), (long)number);
            total += number;
        }
    }

    CLI_LIB_PrintStr_2("%-15s %d\n", "Total", total);
    CLI_LIB_PrintStr_2("%-15s %d\n", "FIB", fib_number);

    return;
}


static void CLI_API_Show_Ip_Route_Detail(NSM_MGR_RouteType_T type, BOOL_T database)
{
    NSM_MGR_GetNextRouteEntry_T entry;
    UI8_T tmp_str[CLI_DEF_MAX_BUFSIZE];
    UI8_T zero_bytes[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    int len;
    BOOL_T first;
    BOOL_T candidate_default;
    UI32_T ret;

    memset(&entry, 0, sizeof(NSM_MGR_GetNextRouteEntry_T));

    for(ret=NSM_PMGR_GetNextRoute(&entry);TRUE;ret=NSM_PMGR_GetNextRoute(&entry))
    {
        UI8_T idx;

        /* Filter type */
        if (type != NSM_MGR_ROUTE_TYPE_IPV4_ALL)
        {
            if (entry.data.ip_route_type != type)
            {
                if (ret == NSM_TYPE_RESULT_OK)
                    continue;
                else if (ret == NSM_TYPE_RESULT_EOF)
                    break;

                break;
            }
        }

        if (ret == NSM_TYPE_RESULT_OK || ret == NSM_TYPE_RESULT_EOF)
        {
            first = TRUE;
            candidate_default = FALSE;

            for(idx=0; idx<entry.data.num_of_next_hop; idx++)
            {
                /* Display forwarding table information only unless database
                 * option is specified.
                 */
                if ((database == FALSE) &&
                    (!(entry.data.next_hop_flags[idx] & NSM_TYPE_NEXTHOP_FLAG_FIB)))
                {
                    continue;
                }

                if (TRUE == first)
                {
                    if ((memcmp(entry.data.ip_route_dest, zero_bytes, SYS_ADPT_IPV4_ADDR_LEN)==0) &&
                            (entry.data.ip_route_dest_prefix_len == 0) &&
                            (database == FALSE))
                    {
                        candidate_default = TRUE;
                    }

                    CLI_LIB_PrintStr_3("%s%c%s ", 
                                    CLI_API_IP_RouteTypeChar(entry.data.ip_route_type),
                                    candidate_default ? '*' : ' ',
                                    CLI_API_IP_RouteSubTypeStr(entry.data.ip_route_subtype));
                    len = SYSFUN_Sprintf(tmp_str, "%s%c%s ", 
                                    CLI_API_IP_RouteTypeChar(entry.data.ip_route_type),
                                    candidate_default ? '*' : ' ',
                                    CLI_API_IP_RouteSubTypeStr(entry.data.ip_route_subtype));

                    if (database == TRUE)
                    {
                        CLI_LIB_PrintStr_3("%c%c%c", 
                            (entry.data.next_hop_flags[idx] & NSM_TYPE_NEXTHOP_FLAG_FIB)
                            ? '*' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_SELECTED)
                            ? '>' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_STALE)
                            ? 'p' : ' ');
                        len += SYSFUN_Sprintf(tmp_str, "%c%c%c", 
                            (entry.data.next_hop_flags[idx] & NSM_TYPE_NEXTHOP_FLAG_FIB)
                            ? '*' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_SELECTED)
                            ? '>' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_STALE)
                            ? 'p' : ' ');
                    }
                    else
                    {
                        CLI_LIB_PrintStr("   ");
                        len += 3;
                    }
                    CLI_LIB_PrintStr_4("%d.%d.%d.%d/", 
                                entry.data.ip_route_dest[0], entry.data.ip_route_dest[1], 
                                entry.data.ip_route_dest[2], entry.data.ip_route_dest[3]);
                    len += SYSFUN_Sprintf(tmp_str, "%d.%d.%d.%d/",
                                entry.data.ip_route_dest[0], entry.data.ip_route_dest[1], 
                                entry.data.ip_route_dest[2], entry.data.ip_route_dest[3]);
                    
                    CLI_LIB_PrintStr_1("%d", entry.data.ip_route_dest_prefix_len);
                    len += SYSFUN_Sprintf(tmp_str, "%d", entry.data.ip_route_dest_prefix_len);

                    first = FALSE;
                }
                else
                {
                    if (database == TRUE)
                    {
                        CLI_LIB_PrintStr_3("     %c%c%c", 
                            (entry.data.next_hop_flags[idx] & NSM_TYPE_NEXTHOP_FLAG_FIB)
                            ? '*' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_SELECTED)
                            ? '>' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_STALE)
                            ? 'p' : ' ');
                    }
                    else
                    {
                        CLI_LIB_PrintStr("        ");
                    }
                    
                    SYSFUN_Sprintf(tmp_str, "%*c", len - 8, ' ');
                    CLI_LIB_PrintStr(tmp_str);
                }
                
                if (entry.data.ip_route_type != NSM_MGR_ROUTE_TYPE_IPV4_LOCAL)
                    CLI_LIB_PrintStr_2("[%d/%ld]", entry.data.distance, (long)entry.data.metric);
                
                switch (entry.data.next_hop_type[idx])
                {
                    case NSM_TYPE_NEXTHOP_TYPE_IPV4:
                    case NSM_TYPE_NEXTHOP_TYPE_IPV4_IFNAME:
                    case NSM_TYPE_NEXTHOP_TYPE_IPV4_IFINDEX:
                        CLI_LIB_PrintStr_4(" via %d.%d.%d.%d", 
                                entry.data.ip_next_hop[idx][1], entry.data.ip_next_hop[idx][1], 
                                entry.data.ip_next_hop[idx][2], entry.data.ip_next_hop[idx][3]);

                        if (entry.data.flags & NSM_TYPE_ROUTE_BLACKHOLE)
                            CLI_LIB_PrintStr(", Null0");
                        else if (entry.data.next_hop_ifname[idx][0] != '\0')
                        {
                            CLI_LIB_PrintStr_1(", %s", entry.data.next_hop_ifname[idx]);
                            entry.data.next_hop_ifname[idx][0] = '\0';
                        }
                        else if (entry.data.next_hop_ifindex[idx])
                        {
                            CLI_LIB_PrintStr_1(", %s", entry.data.next_hop_ifname[idx]);
                        }
                        
                        break;

                    case NSM_TYPE_NEXTHOP_TYPE_IFINDEX:
                        CLI_LIB_PrintStr_1(" is directly connected, %s", entry.data.next_hop_ifname[idx]);
                        entry.data.next_hop_ifname[idx][0] = '\0';
                        break;

                    case NSM_TYPE_NEXTHOP_TYPE_IFNAME:
                        CLI_LIB_PrintStr_1(" is directly connected, %s", entry.data.next_hop_ifname[idx]);
                        entry.data.next_hop_ifname[idx][0] = '\0';
                        break;

                    default:
                        break;
                }

                if ((entry.data.next_hop_flags[idx] & NSM_TYPE_NEXTHOP_FLAG_ACTIVE) != 
                                        NSM_TYPE_NEXTHOP_FLAG_ACTIVE)
                {
                    CLI_LIB_PrintStr(" inactive");
                }

                CLI_LIB_PrintStr("\n");
            }
        }

        if(ret != NSM_TYPE_RESULT_OK)
            break;
    }

    return;
}


UI32_T CLI_API_L3_Show_Ip_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    const char show_header[] =
        "Codes: C - connected, S - static, R - RIP, B - BGP\n"
        "       O - OSPF, IA - OSPF inter area\n"
        "       N1 - OSPF NSSA external type 1, N2 - OSPF NSSA external type 2\n"
        "       E1 - OSPF external type 1, E2 - OSPF external type 2\n"
        "       i - IS-IS, L1 - IS-IS level-1, L2 - IS-IS level-2,"
        " ia - IS-IS inter area\n";
    /* Normal case.  */
    const char header_normal[] =
        "       * - candidate default\n\n";

    /* Database case.  */
    const char header_database[] =
        "       > - selected route, * - FIB route, p - stale info\n\n";

    BOOL_T database = FALSE;
    NSM_MGR_RouteType_T type = NSM_MGR_ROUTE_TYPE_IPV4_ALL;
    BOOL_T summary = FALSE;

    if (arg[0] != NULL)
    {
        switch(*arg[0])
        {
            case 'd':
            case 'D':
                database = TRUE;
                break;

            case 'c':
            case 'C':
                type = NSM_MGR_ROUTE_TYPE_IPV4_LOCAL;
                break;

            case 's':
            case 'S':
                if ((arg[0][1] == 't') || (arg[0][1] == 'T'))
                    type = NSM_MGR_ROUTE_TYPE_IPV4_STATIC;
                else if ((arg[0][1] == 'u') || (arg[0][1] == 'U'))
                    summary = TRUE;
                else
                {
                    return CLI_ERR_INTERNAL;
                }
                break;

            default:
                break;
        }
    }

    if (summary == TRUE)
    {
        CLI_API_Show_Ip_Route_Summary();
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr(show_header);
    if (database == TRUE)
        CLI_LIB_PrintStr(header_database);
    else
        CLI_LIB_PrintStr(header_normal);

    CLI_API_Show_Ip_Route_Detail(type, database);

    return CLI_NO_ERROR;
}

#endif

/* command: clear arp-cache */
UI32_T CLI_API_Clear_Arp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   char choice[2] = {0};

   CLI_LIB_PrintStr("This operation will delete all the dynamic entries in ARP Cache.\r\n");
   CLI_LIB_PrintStr("Do you want to continue this operation (y/n)?");
   CLI_PARS_ReadLine(choice, sizeof(choice), TRUE, FALSE);
   CLI_LIB_PrintNullStr(1);

   if( choice[0] == 0 || (choice[0] != 'y' && choice[0] != 'Y') )
      return CLI_NO_ERROR;

   if ( NETCFG_PMGR_ND_DeleteAllDynamicIpv4NetToMediaEntry()!= NETCFG_TYPE_OK)
   {
      CLI_LIB_PrintStr("Failed flush the dynamic ARP Cache.\r\n");
   }
   return CLI_NO_ERROR;
}

/* command: show arp */
UI32_T CLI_API_Show_Arp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;

    NETCFG_TYPE_IpNetToMediaEntry_T  entry;

    char  type[30] = {0};
    UI32_T vid = 0;
    UI32_T time_out_value = 0;
    UI8_T ip_str[18] = {0};
    UI32_T count = 0;
    UI32_T buff_len = 0;

    memset(&entry, 0, sizeof(NETCFG_TYPE_IpNetToMediaEntry_T));

    if(NETCFG_POM_ND_GetIpNetToMediaTimeout(&time_out_value) != FALSE )
    {
      CLI_LIB_PrintStr_1("ARP Cache Timeout: %lu (seconds)\r\n", (unsigned long)time_out_value);
    }
    sprintf(buff,"\r\nIP Address      MAC Address       Type      Interface\r\n");
    PROCESS_MORE(buff);
    sprintf(buff,"--------------- ----------------- --------- -----------\r\n");
    PROCESS_MORE(buff);


    while (NETCFG_PMGR_ND_GetNextIpNetToMediaEntry(&entry) == NETCFG_TYPE_OK)
   //while (NETCFG_PMGR_ARP_GetNextIpNetToMediaEntry(&entry) == NETCFG_TYPE_OK)
    {

        L_INET_Ntoa(entry.ip_net_to_media_net_address,ip_str);

        switch(entry.ip_net_to_media_type)
        {
            case VAL_ipNetToMediaType_other:
            strcpy(type,"other");
            break;

            case VAL_ipNetToMediaType_invalid:
            strcpy(type,"invalid");
            break;

            case VAL_ipNetToMediaType_dynamic:
            strcpy(type,"dynamic");
            break;

            case VAL_ipNetToMediaType_static:
            strcpy(type,"static");
            break;

            default:
            strcpy(type,"other");
            break;
        }

        buff_len = sprintf(buff, "%-15s %02X-%02X-%02X-%02X-%02X-%02X %-9s",
            ip_str,
            entry.ip_net_to_media_phys_address.phy_address_cctet_string[0],
            entry.ip_net_to_media_phys_address.phy_address_cctet_string[1],
            entry.ip_net_to_media_phys_address.phy_address_cctet_string[2],
            entry.ip_net_to_media_phys_address.phy_address_cctet_string[3],
            entry.ip_net_to_media_phys_address.phy_address_cctet_string[4],
            entry.ip_net_to_media_phys_address.phy_address_cctet_string[5],
            type);
#if (SYS_CPNT_CRAFT_PORT == TRUE)
        if(SYS_ADPT_CRAFT_INTERFACE_IFINDEX == entry.ip_net_to_media_if_index)
        {
            sprintf(buff + buff_len, " Craft\r\n");
        }
        else
#endif /* SYS_CPNT_CRAFT_PORT */
        {
            VLAN_OM_ConvertFromIfindex(entry.ip_net_to_media_if_index, &vid);
            sprintf(buff + buff_len, " VLAN %-4lu\r\n", (unsigned long)vid);
        }

        PROCESS_MORE(buff);
        count++;
    }

    sprintf(buff, "\r\nTotal entry : %lu\r\n",(unsigned long)count);
    PROCESS_MORE(buff);
    return CLI_NO_ERROR;

}

/* command : arp timeout */
UI32_T CLI_API_Arp_Timeout(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ARP == TRUE)
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_ARP_TIMEOUT:
        if(NETCFG_PMGR_ND_SetIpNetToMediaTimeout(atoi((char *)arg[0]))!=NETCFG_TYPE_OK)
        {
            CLI_LIB_PrintStr("Failed to configure the timeout.\r\n");
            return CLI_NO_ERROR;
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_ARP_TIMEOUT:

        if(NETCFG_PMGR_ND_SetIpNetToMediaTimeout(SYS_DFLT_IP_NET_TO_MEDIA_ENTRY_TIMEOUT)!=NETCFG_TYPE_OK)
        {
            CLI_LIB_PrintStr("Failed to set the timeout to default.\r\n");
            return CLI_NO_ERROR;
        }
        break;
    }
#endif
    return CLI_NO_ERROR;
}

#endif

/* command: [no] ecmp load-balance
 *               { dst-ip-l4-port | hash-selection-list hsl-id }
 */
UI32_T CLI_API_Ecmp_Load_Balance(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
    UI32_T  mode = SYS_DFLT_ECMP_BALANCE_MODE, hidx =0;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_ECMP_LOADBALANCE:
        if ((arg[0][0] == 'h') || (arg[0][0] == 'H'))
        {
            mode = NETCFG_TYPE_ECMP_HASH_SELECTION;
            hidx = atoi(arg[1]);
        }
        break;
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_ECMP_LOADBALANCE:
        break;
    default:
        return CLI_NO_ERROR;
    }

    if (NETCFG_TYPE_OK != NETCFG_PMGR_ROUTE_SetEcmpBalanceMode(mode, hidx))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set ECMP load balance mode.\r\n");
#endif
    }

#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

    return CLI_NO_ERROR;
}

/* command: show ecmp load-balance */
UI32_T CLI_API_L3_Show_Ecmp_Load_Balance(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
    char *ecmp_tag = "ECMP Load Balance Mode";

    UI32_T  mode, idx;

    if (NETCFG_TYPE_OK != NETCFG_POM_ROUTE_GetEcmpBalanceMode(&mode, &idx))
    {
        mode = NETCFG_TYPE_ECMP_MAX;
    }

    switch(mode)
    {
    case NETCFG_TYPE_ECMP_HASH_SELECTION:
        CLI_LIB_Printf(" %s : %s (%ld)\r\n",ecmp_tag, "Hash Selection List", (long)idx);
        break;
    case NETCFG_TYPE_ECMP_DIP_L4_PORT:
        CLI_LIB_Printf(" %s : %s\r\n", ecmp_tag, "Destination IP Address And L4 Port");
        break;
#if 0
    case NETCFG_TYPE_ECMP_DST_IP:
        CLI_LIB_Printf(" %s : %s", ecmp_tag, "Destination IP Address");
        break;
#endif
    default:
        CLI_LIB_PrintStr("Failed to display ECMP load balance information.\r\n");
        break;
    }

#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

   return CLI_NO_ERROR;
}
