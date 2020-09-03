#include <string.h>
#include "leaf_2096.h"
#include "cli_api.h"
#include "cli_api_l3.h"
#include "cli_lib.h"
#include "leaf_es3626a.h"
#include "vlan_lib.h"
#include "l_prefix.h"
#include "l_inet.h"
#include "leaf_4001.h"



#include "l_inet.h"
#include "netcfg_type.h"
#include "netcfg_pmgr_nd.h"
#include "netcfg_pom_nd.h"
#include "netcfg_mgr_nd.h"
#include "netcfg_pmgr_ip.h"
#include "netcfg_pom_ip.h"
#include "netcfg_pmgr_route.h"

#include "sys_dflt.h"  /* for SYS_DFLT_IPV6_INTERFACE_MTU, why can't ?*/
#include "ip_lib.h"

#include "nsm_pmgr.h"

#include "cli_lib.h"





/*Simon's debug function*/
#define DEBUG_FLAG_BIT_DEBUG 0x01
#define DEBUG_FLAG_BIT_INFO  0x02
#define UNUSED __attribute__ ((__unused__))

/***************************************************************/
static UI32_T UNUSED DEBUG_FLAG = 0;//DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO|DEBUG_FLAG_BIT_NOTE;
/***************************************************************/

#define DBGprintf(format,args...) ((DEBUG_FLAG_BIT_DEBUG & DEBUG_FLAG)==0)?:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define INFOprintf(format,args...) ((DEBUG_FLAG_BIT_INFO & DEBUG_FLAG)==0)?:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define NOTEprintf(format,args...) ((DEBUG_FLAG_BIT_NOTE & DEBUG_FLAG)==0)?:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)

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
/*END Simon's debug function*/

UI32_T CLI_API_Interface_Tunnel(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    UI32_T tid;

    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;
    tid = (UI32_T)atoi((char*)arg[0]);
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_INTERFACE_TUNNEL:
            if (NETCFG_PMGR_IP_CreateTunnelInterface(tid) != NETCFG_TYPE_OK)
            {
                 CLI_LIB_PrintStr("Fail to create interface.\r\n");
                return CLI_NO_ERROR;
            }
            IP_LIB_ConvertTunnelIdToIfindex( tid, &ctrl_P->CMenu.tunnel_ifindex);
            ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_INTERFACE_TUNNEL_MODE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_INTERFACE_TUNNEL:
            if (NETCFG_PMGR_IP_DeleteTunnelInterface(tid) != NETCFG_TYPE_OK)
            {
                 CLI_LIB_PrintStr("Fail to delete interface.\r\n");
                 return CLI_NO_ERROR;
            }
            break;

        default:
            return CLI_ERR_CMD_INVALID;
    }
#endif

    return CLI_NO_ERROR;
}
UI32_T CLI_API_Tunnel_Ipv6Address(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IP_TUNNEL == TRUE && SYS_CPNT_IPV6 == TRUE)

    UI32_T cli_ifindex;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    UI32_T ipv6_addr_type;
    UI32_T rc_netcfg;

    cli_ifindex = ctrl_P->CMenu.tunnel_ifindex;
    ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_TUNNEL_CMD_W2_IPV6_ADDRESS:
                 
            memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                               arg[0],
                                                               (L_INET_Addr_T *)&rif_config.addr,
                                                               sizeof(rif_config.addr)))
                CLI_LIB_PrintStr("Invalid IPv6 address format.\r\n");

            if(arg[1] == NULL)
            {
                /* check link-local address */
                if(L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL(rif_config.addr.addr))
                {
                    CLI_LIB_PrintStr("Please use: ""IPv6 address X:X:X:X::X link-local"" instead \r\n");
                    return CLI_NO_ERROR;
                    ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL;
                }
                
            }
            else if((arg[1][0] == 'l') || (arg[1][0] == 'L')) /* link-local */
            {
                    ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL;
                /* check link-local address */
                if(!L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL_UNICAST(rif_config.addr.addr))
                {
                    CLI_LIB_PrintStr("Invalid link-local address.\r\n");
                    return CLI_NO_ERROR;
                }
                /* assign zoneid */
                IP_LIB_ConvertZoneIDFromTunnelIfindex(cli_ifindex, &rif_config.addr.zoneid);
            }

            else if((arg[1][0] == 'e') || (arg[1][0] == 'E')) /* eui-64 */
            {
                /* check link-local address */
                if(L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL(rif_config.addr.addr))
                {
                    CLI_LIB_PrintStr("Please use: ""IPv6 address X:X:X:X::X link-local"" instead \r\n");
                    return CLI_NO_ERROR;
                }
                ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_EUI64;
            }
            else
            {
                CLI_LIB_PrintStr("Invalid IPv6 address format.\r\n");
            }

            rif_config.ifindex = cli_ifindex;
            rif_config.ipv6_addr_config_type = NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_MANUAL;
            rif_config.ipv6_addr_type = ipv6_addr_type;
            rif_config.row_status = VAL_netConfigStatus_2_createAndGo;

            if(rif_config.ipv6_addr_type == NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL)
                rif_config.addr.preflen = 64;

            rc_netcfg = NETCFG_PMGR_IP_SetTunnelIPv6Rif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
            switch(rc_netcfg)
            {
                case NETCFG_TYPE_OK:
                    break;
                case NETCFG_TYPE_INVALID_IPV6_UNSPECIFIED:
                    CLI_LIB_PrintStr("Unspecified address is not allowed.\r\n");
                    return CLI_NO_ERROR;
                case NETCFG_TYPE_INVALID_IPV6_LOOPBACK:
                    CLI_LIB_PrintStr("Loopback address is not allowed.\r\n");
                    return CLI_NO_ERROR;
                case NETCFG_TYPE_INVALID_IPV6_MULTICAST:
                    CLI_LIB_PrintStr("IPv6 multicast address is not allowed.\r\n");
                    return CLI_NO_ERROR;
                case NETCFG_TYPE_TUNNEL_SOURCE_VLAN_NOT_EXIST:
                    CLI_LIB_PrintStr("Tunnel source VLAN not exist.\r\n");
                    return CLI_NO_ERROR;
                default:
                    /* unknown error */
                    CLI_LIB_PrintStr("Failed to add IPv6 address.\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_TUNNEL_CMD_W3_NO_IPV6_ADDRESS:
            memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                               arg[0],
                                                               (L_INET_Addr_T *)&rif_config.addr,
                                                               sizeof(rif_config.addr)))
                CLI_LIB_PrintStr("Invalid IPv6 address format.\r\n");


            if(arg[1] == NULL)
            {
                /* check link-local address */
                if(L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL(rif_config.addr.addr))
                {
                    ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL;
                } 
            }
            else if((arg[1][0] == 'l') || (arg[1][0] == 'L')) /* link-local */
            {
                ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL;

                /* check link-local address */
                if(!L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL_UNICAST(rif_config.addr.addr))
                {
                    CLI_LIB_PrintStr("Invalid link-local address.\r\n");
                    return CLI_NO_ERROR;
                }
                /* assign zoneid */
                IP_LIB_ConvertZoneIDFromTunnelIfindex(cli_ifindex, &rif_config.addr.zoneid);
             
            }
            else if((arg[1][0] == 'e') || (arg[1][0] == 'E')) /* eui-64 */
            {
                ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_EUI64;
            }
            else
            {
                CLI_LIB_PrintStr("Invalid IPv6 address format.\r\n");
            }

            rif_config.ifindex = cli_ifindex;
            rif_config.ipv6_addr_config_type = NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_MANUAL;
            rif_config.ipv6_addr_type = ipv6_addr_type;

            if(rif_config.addr.type == L_INET_ADDR_TYPE_IPV6Z)
                rif_config.addr.preflen = 64;

            rif_config.row_status = VAL_netConfigStatus_2_destroy;
            rc_netcfg = NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
            switch(rc_netcfg)
            {
                case NETCFG_TYPE_ENTRY_NOT_EXIST:
                    CLI_LIB_PrintStr("Address not found.\r\n");
                case NETCFG_TYPE_FAIL:
                    CLI_LIB_PrintStr("Failed to delete IPv6 address.\r\n");
                    return CLI_NO_ERROR;
                case NETCFG_TYPE_OK:
                    return CLI_NO_ERROR;
                default:
                    break;
            }
            break;

        default:
            CLI_LIB_PrintStr_1("(no) IPv6 address: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif /* #if (SYS_CPNT_IP_TUNNEL == TRUE && SYS_CPNT_IPV6 == TRUE) */

    return (UI32_T)CLI_NO_ERROR;
}
UI32_T CLI_API_Tunnel_Source(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    UI32_T cli_ifindex;
    UI32_T src_vid;
    UI32_T rc;

    cli_ifindex = ctrl_P->CMenu.tunnel_ifindex;
    if((arg[0][0] != 'v' && arg[0][0] != 'V') )
        return CLI_ERR_CMD_UNRECOGNIZED;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_TUNNEL_CMD_W2_TUNNEL_SOURCE:
            src_vid =   (UI32_T)atoi((char*)arg[1]);
            if(!VLAN_POM_IsVlanExisted(src_vid))
            {
                CLI_LIB_PrintStr("Source vlan do not exist.\r\n");
                return CLI_NO_ERROR;
            }
            rc = NETCFG_PMGR_IP_SetTunnelSourceVLAN(cli_ifindex, src_vid);
            switch(rc)
            {
                case NETCFG_TYPE_OK:
                    break;
                case NETCFG_TYPE_INTERFACE_NOT_EXISTED:
                    CLI_LIB_PrintStr("Source vlan do not exist.\r\n");
                    break;
                case NETCFG_TYPE_TUNNEL_SOURCE_VLAN_HAS_NO_IPV4_ADDRESS:
                    CLI_LIB_PrintStr("Source vlan do not have IPv4 address.\r\n");
                    break;
                default:
                    /* unknown error */
                    CLI_LIB_PrintStr("Failed to set tunnel source vlan.\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_TUNNEL_CMD_W3_NO_TUNNEL_SOURCE:

            
            rc = NETCFG_PMGR_IP_UnsetTunnelSourceVLAN(cli_ifindex);
            switch(rc)
            {
                case NETCFG_TYPE_OK:
                    break;
                case NETCFG_TYPE_INTERFACE_NOT_EXISTED:
                    CLI_LIB_PrintStr("Source VLAN do not exist.\r\n");
                    break;
                case NETCFG_TYPE_TUNNEL_SOURCE_VLAN_HAS_NO_IPV4_ADDRESS:
                    CLI_LIB_PrintStr("Source VLAN do not have IPv4 address.\r\n");
                    break;
                default:
                    /* unknown error */
                    CLI_LIB_PrintStr("Failed to set tunnel source vlan.\r\n");
            }
            break;

        default:
            CLI_LIB_PrintStr_1("(no) tunnel source address: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif
	
    return (UI32_T)CLI_NO_ERROR;
}
UI32_T CLI_API_Tunnel_Deatination(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    UI32_T cli_ifindex;
    UI32_T rc;
    L_INET_AddrIp_T address;
    cli_ifindex = ctrl_P->CMenu.tunnel_ifindex;
    

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_TUNNEL_CMD_W2_TUNNEL_DESTINATION:
            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                               arg[0],
                                                               (L_INET_Addr_T *)&address,
                                                               sizeof(address)))
           {
                DBGprintf("conver fail!");
                return CLI_ERR_CMD_INVALID;
           }
            rc = NETCFG_PMGR_IP_SetTunnelDestination(cli_ifindex, &address);
            switch(rc)
            {
                case NETCFG_TYPE_OK:
                    break;
                case NETCFG_TYPE_INTERFACE_NOT_EXISTED:
                    CLI_LIB_PrintStr("Source VLAN do not exist.\r\n");
                    break;
                case NETCFG_TYPE_TUNNEL_SOURCE_VLAN_HAS_NO_IPV4_ADDRESS:
                    CLI_LIB_PrintStr("Source VLAN do not have IPv4 address.\r\n");
                    break;
                case NETCFG_TYPE_TUNNEL_NOT_CONFIGURED_TUNNEL:
                    CLI_LIB_PrintStr("Not Configured Tunnel.\r\n");
                    break;
                default:
                    /* unknown error */
                    CLI_LIB_PrintStr("Failed to set tunnel destination.\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_TUNNEL_CMD_W3_NO_TUNNEL_DESTINATION:

            
            rc = NETCFG_PMGR_IP_UnsetTunnelDestination(cli_ifindex);
            switch(rc)
            {
                case NETCFG_TYPE_OK:
                    break;
                case NETCFG_TYPE_INTERFACE_NOT_EXISTED:
                    CLI_LIB_PrintStr("Source VLAN do not exist.\r\n");
                    break;
                case NETCFG_TYPE_TUNNEL_SOURCE_VLAN_HAS_NO_IPV4_ADDRESS:
                    CLI_LIB_PrintStr("Source VLAN do not have IPv4 address.\r\n");
                    break;
                case NETCFG_TYPE_TUNNEL_NOT_CONFIGURED_TUNNEL:
                    CLI_LIB_PrintStr("Not Configured Tunnel.\r\n");
                    break;
                default:
                    /* unknown error */
                    CLI_LIB_PrintStr("Failed to set tunnel destination.\r\n");
            }
            break;

        default:
            CLI_LIB_PrintStr_1("(no) tunnel source address: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif
	
    return (UI32_T)CLI_NO_ERROR;
}
UI32_T CLI_API_Tunnel_Nd(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    CLI_LIB_PrintStr("Cmd move to ipv6->nd->suppress\r\n");
    return 1;
}
UI32_T CLI_API_Tunnel_Ttl(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    UI32_T cli_ifindex, ttl, rc; 
    cli_ifindex = ctrl_P->CMenu.tunnel_ifindex;
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_TUNNEL_CMD_W2_TUNNEL_TTL:
            ttl = atoi(arg[0]);
            if(NETCFG_TYPE_OK!=(rc=NETCFG_PMGR_IP_SetTunnelTtl( cli_ifindex,  ttl)))
                CLI_LIB_PrintStr("Fail to set tunnel TTL\r\n");
            break;
        case PRIVILEGE_CFG_INTERFACE_TUNNEL_CMD_W3_NO_TUNNEL_TTL:
            if(NETCFG_TYPE_OK!=(rc=NETCFG_PMGR_IP_UnsetTunnelTtl( cli_ifindex)))
                CLI_LIB_PrintStr("Fail to set no tunnel  TTL\r\n");
            break;
    }
#endif

    return (UI32_T)CLI_NO_ERROR;
}
UI32_T  CLI_API_Tunnel_Mode(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    UI32_T cli_ifindex;
    UI32_T rc;
    int tunnel_mode;

    cli_ifindex = ctrl_P->CMenu.tunnel_ifindex;
    /*only support "ipv6ip"*/
    if(arg[0][0]!='i' && arg[0][0]!='I' )
    { 
        DBGprintf("something wrong: %s", arg[0]);
        return CLI_ERR_CMD_INVALID;
    }
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_TUNNEL_CMD_W2_TUNNEL_MODE:
                switch (arg[1][0])
                {
                    case '6':/*6to4*/
                        tunnel_mode = NETCFG_TYPE_TUNNEL_MODE_6TO4;
                        break;
                    case 'i':
                    case 'I':/*ISATAP*/
                        tunnel_mode = NETCFG_TYPE_TUNNEL_MODE_ISATAP;
                        break;
                    case 'C':/*configured*/
                    case 'c':
                        tunnel_mode = NETCFG_TYPE_TUNNEL_MODE_CONFIGURED;
                        break;
                    default:
                        CLI_LIB_PrintStr("Unknown command parameter\r\n");
                        return CLI_ERR_CMD_UNRECOGNIZED; 
                }
            rc = NETCFG_PMGR_IP_SetTunnelMode(cli_ifindex, tunnel_mode);
            switch (rc)
            {
            case NETCFG_TYPE_TUNNEL_TUNNEL_DEST_MUST_UNCONFIGURED:
                CLI_LIB_PrintStr("Tunnel destination must unconfigured\r\n");
                break;
            case NETCFG_TYPE_FAIL:
                CLI_LIB_PrintStr("Fail to set tunnel mode\r\n");
                break;
            default:
                break;
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_TUNNEL_CMD_W3_NO_TUNNEL_MODE:
            rc = NETCFG_PMGR_IP_UnsetTunnelMode(cli_ifindex);
            switch (rc)
            {
            case NETCFG_TYPE_TUNNEL_TUNNEL_DEST_MUST_UNCONFIGURED:
                CLI_LIB_PrintStr("Tunnel destination must unconfigured\r\n");
                break;
            case NETCFG_TYPE_FAIL:
                CLI_LIB_PrintStr("Fail to no tunnel mode\r\n");
                break;
            default:
                break;
            }
            break;
        default:
            CLI_LIB_PrintStr_1("(no) tunnel mode: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif

    return (UI32_T)CLI_NO_ERROR;
}

UI32_T  CLI_API_ShowIpv6Tunnel(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    NETCFG_TYPE_L3_Interface_T intf;
    NETCFG_TYPE_Tunnel_Interface_T *tunnel_if;
    NETCFG_TYPE_InetRifConfig_T rif;
    UI32_T tid,vid;
    char str_tunnel_mode[4][20]={"unspecified","Configured","6-to-4","ISATAP"};
    char vid_string[12];
    char src_addr_buf[64] = {0};
    char dst_addr_buf[64] = {0};
    CLI_LIB_PrintStr("\r\n");
    memset(&intf,0,sizeof(intf));
    while(NETCFG_TYPE_OK == NETCFG_POM_IP_GetNextL3Interface(&intf))
    {
        if(intf.iftype !=VLAN_L3_IP_TUNNELTYPE)
            continue;
        IP_LIB_ConvertTunnelIdFromIfindex(intf.ifindex, &tid);
        tunnel_if=&intf.u.tunnel_intf;
        memset(&rif,0,sizeof(rif));
        rif.ifindex = tunnel_if->src_vid_ifindex;
        NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif);
        vid =0;
        VLAN_OM_ConvertFromIfindex(tunnel_if->src_vid_ifindex, &vid);
        vid_string[0]=0;
        if(vid)
        {
            sprintf(vid_string, "[VLAN%ld]",vid);
        }
        
        if(tunnel_if->tunnel_mode == NETCFG_TYPE_TUNNEL_MODE_CONFIGURED)
        {
            sprintf(src_addr_buf, "%d.%d.%d.%d", EXPAND_IPV4(rif.addr.addr));
            sprintf(dst_addr_buf, "%d.%d.%d.%d", EXPAND_IPV4(tunnel_if->dip.addr));
        }
        else if(tunnel_if->tunnel_mode == NETCFG_TYPE_TUNNEL_MODE_6TO4 
                    ||tunnel_if->tunnel_mode == NETCFG_TYPE_TUNNEL_MODE_ISATAP)
        {
            sprintf(src_addr_buf, "%d.%d.%d.%d", EXPAND_IPV4(rif.addr.addr));
            sprintf(dst_addr_buf,"Dynamic");
        }

        
        CLI_LIB_PrintStr_1("Tunnel:%ld \r\n", tid);
        CLI_LIB_PrintStr_1(" Tunnel Current State                       : %s \r\n", (tunnel_if->row_status==VAL_netConfigStatus_2_active)?"Up":"Down");
        CLI_LIB_PrintStr_2(" Tunnel Source Address                      : %s%s\r\n",vid_string,src_addr_buf);
        CLI_LIB_PrintStr_1(" Tunnel Destination Address                 : %s\r\n",dst_addr_buf);
        if(tunnel_if->ttl == 0)
            CLI_LIB_PrintStr(" Time to Live                               : 0(Hop Limit in IPv6 Header)\r\n");
        else
        CLI_LIB_PrintStr_1(" Time to Live                               : %d\r\n", tunnel_if->ttl);
        CLI_LIB_PrintStr_1(" Tunnel Mode (Configured / 6-to-4)          : %s\r\n" , str_tunnel_mode[tunnel_if->tunnel_mode]);
        CLI_LIB_PrintStr  ("\r\n");
        /*
        Number of received tunnel error packets    : 0
        Number of received tunnel packets          : 12123
        Number of transmitted tunnel errors        : 0
        Number of transmitted tunnel packets       : 12118
        */

    }
#endif
	
    return (UI32_T)CLI_NO_ERROR;
}

