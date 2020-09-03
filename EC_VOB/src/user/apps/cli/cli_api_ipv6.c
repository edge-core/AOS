#include "sys_cpnt.h"

#if (SYS_CPNT_IPV6 == TRUE)
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
#include "netcfg_om_nd.h"

#include "netcfg_pmgr_ip.h"
#include "netcfg_pom_ip.h"
#include "netcfg_pmgr_route.h"

#include "sys_dflt.h"  /* for SYS_DFLT_IPV6_INTERFACE_MTU, why can't ?*/
#include "ip_lib.h"
#if (SYS_CPNT_NSM == TRUE)
#include "nsm_mgr.h"
#include "nsm_pmgr.h"
#endif

#include "ipal_if.h"
#include "ipal_types.h"
#include "cli_lib.h"

#if (SYS_CPNT_PING == TRUE)
#include "leaf_2925p.h" /* ping MIB */
#include "ping_type.h"  /* ping MIB */
#include "ping_pmgr.h"  /* ping MIB */
#include "ping_pom.h"   /* ping MIB */
#include "ip_lib.h"     /* ping MIB */
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
#include "traceroute_pmgr.h"
#include "traceroute_pom.h"
#endif

#if (SYS_CPNT_DNS == TRUE)
#include "dns_pmgr.h"
#endif

#include "cmgr.h"

#define CHECK_FLAG(V,F)      ((V) & (F))

#define SN_LAN                          1

/*Simon's debug function*/
#define DEBUG_FLAG_BIT_DEBUG 0x01
#define DEBUG_FLAG_BIT_INFO  0x02
#define DEBUG_FLAG_BIT_NOTE  0x04
#define UNUSED __attribute__ ((__unused__))

/***************************************************************/
static UI32_T UNUSED DEBUG_FLAG = 0;//DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO|DEBUG_FLAG_BIT_NOTE;
/***************************************************************/

#define DBGprintf(format,args...) if((DEBUG_FLAG_BIT_DEBUG & DEBUG_FLAG)!=0){printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args);}
#define INFOprintf(format,args...) if((DEBUG_FLAG_BIT_INFO & DEBUG_FLAG)!=0){printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args);}
#define NOTEprintf(format,args...) if((DEBUG_FLAG_BIT_NOTE & DEBUG_FLAG)!=0){printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args);}

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

#if (SYS_CPNT_DNS == TRUE)
// static BOOL_T cli_api_hoststring2inet(UI8_T *hoststr_p, L_INET_AddrIp_T *inet_addr_p);
#endif  /* #if (SYS_CPNT_DNS == TRUE) */


UI32_T  CLI_API_IPV6_Neighbor(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_ROUTING == TRUE)

    L_INET_AddrIp_T    ipv6_addr;

    //UI8_T    phy_addr[SYS_ADPT_MAC_ADDR_LEN]    = {0};
    NETCFG_TYPE_PhysAddress_T phy_addr;
    UI32_T ifindex;
    UI32_T result;
    UI32_T vid;

    phy_addr.phy_address_type=SN_LAN;
    phy_addr.phy_address_len =  SYS_ADPT_MAC_ADDR_LEN;


    switch (cmd_idx){
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_IPV6_NEIGHBOR:
            //ipv6-address+interface-type+interface-number+hardware-address
            if(arg[0] == NULL || arg[1] == NULL || arg[2] == NULL || arg[3] == NULL)
            {
                CLI_LIB_Printf("Unknown error!\r\n");
                break;
            }

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_ADDR_RAW,
                                                               arg[0],
                                                               (L_INET_Addr_T *)&ipv6_addr,
                                                               sizeof(ipv6_addr)))
            {
                CLI_LIB_Printf("IPv6 neighbor: Incorrect IPv6 address input!\r\n");
                break;
            }
            if(arg[1][0] != 'V' && arg[1][0] != 'v')
            {
                CLI_LIB_Printf("IPv6 neighbor: Not supported hardware interface type!\r\n");
                break;
            }
            vid =atoi(arg[2]);
            if(FALSE == VLAN_OM_ConvertToIfindex(vid,&ifindex))
            {
                CLI_LIB_Printf("Invalid vid: %2d\r\n",vid);
                break;
            }
            if(FALSE == CLI_LIB_ValsInMac(arg[3], phy_addr.phy_address_cctet_string))
            {
                CLI_LIB_Printf("IPv6 neighbor: incorrect ethernet mac address!\r\n");
                break;
            }
            if (ipv6_addr.type == L_INET_ADDR_TYPE_IPV6Z)
            {
                ipv6_addr.zoneid = vid;
            }
            if(NETCFG_TYPE_OK !=  NETCFG_PMGR_ND_AddStaticIpNetToPhysicalEntry(ifindex,&ipv6_addr,&phy_addr))
            {
                CLI_LIB_Printf("IPv6 neighbor: Add IPv6 neighbor cache entry failed!\r\n");
                break;
            }
            break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IPV6_NEIGHBOR:
             //ipv6-address+interface-type+interface-number
            if(arg[0] == NULL || arg[1] == NULL || arg[2] == NULL)
            {
                CLI_LIB_Printf("Unknown error!\r\n");
                break;
            }

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_ADDR_RAW,
                                                               arg[0],
                                                               (L_INET_Addr_T *)&ipv6_addr,
                                                               sizeof(ipv6_addr)))
            {
                CLI_LIB_Printf("IPv6 neighbor: Incorrect IPv6 address input!\r\n");
                break;
            }
            if(arg[1][0] != 'V' && arg[1][0] != 'v')
            {
                CLI_LIB_Printf("IPv6 neighbor: Not supported hardware interface type!\r\n");
                break;
            }
            vid =atoi(arg[2]);
            if(FALSE == VLAN_OM_ConvertToIfindex(vid,&ifindex))
            {
                CLI_LIB_Printf("Invalid vid: %2d\r\n",vid);
                break;
            }
            if (ipv6_addr.type == L_INET_ADDR_TYPE_IPV6Z)
            {
                ipv6_addr.zoneid = vid;
            }
            result = NETCFG_PMGR_ND_DeleteStaticIpNetToPhysicalEntry(ifindex,&ipv6_addr);
            switch (result)
            {
                case NETCFG_TYPE_OK:
                    break;
                case NETCFG_TYPE_INTERFACE_NOT_EXISTED:
                    CLI_LIB_Printf("Intreface not existed!");
                    break;
                case NETCFG_TYPE_CAN_NOT_DELETE:
                default:
                    CLI_LIB_Printf("no IPv6 address: remove IPv6 address failed!\r\n");
                    break;
            };
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 neighbor: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
        }
#endif /* SYS_CPNT_IPV6_ROUTING */

        return (UI32_T)CLI_NO_ERROR;
}

UI32_T  CLI_API_IPV6_NdPrefix(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    L_INET_AddrIp_T prefix;
    //UI32_T prefix_len;
    UI32_T valid_lifetime, preferred_lifetime;
    BOOL_T          enable_onlink = TRUE, enable_autoconf = TRUE;

    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IPV6_ND_PREFIX:

            //ipv6-prefix+default |valid-lifetime preferred-lifetime|off-link |no-autoconfig
            if(arg[0] == NULL || arg[1] == NULL)
            {
                CLI_LIB_Printf("Unknown error!\r\n");
                break;
            }

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC,
                                                               arg[0],
                                                               (L_INET_Addr_T *)&prefix,
                                                               sizeof(prefix)))
            {
                CLI_LIB_Printf("Incorrect IPv6 address/prefix input!\r\n");
                break;
            }
            if(FALSE == NETCFG_POM_ND_GetNdPrefix(ctrl_P->CMenu.vlan_ifindex, &prefix, &valid_lifetime, &preferred_lifetime,&enable_onlink,&enable_autoconf))
                NETCFG_POM_ND_GetNdDefaultPrefixConfig(&valid_lifetime,&preferred_lifetime,&enable_onlink,&enable_autoconf);

            if(arg[1][0]=='D' || arg[1][0] =='d') /* default */
                NETCFG_POM_ND_GetNdDefaultPrefixConfig(&valid_lifetime,&preferred_lifetime,&enable_onlink,&enable_autoconf);
            else
            {
                int i;

                /* ipv6_ready */
                valid_lifetime      = strtoul(arg[1], (char **)NULL, 10);
                preferred_lifetime  = strtoul(arg[2], (char **)NULL, 10);
                /* DEBUG */
                //printf("valid_lifetime: %lu", valid_lifetime);
                for(i=3; arg[i]; )
                {
                    if (arg[i][0] == 'o') /* offlink */
                    {
                        enable_onlink = FALSE;
                        i++;
                    }
                    else if (arg[i][0] == 'n') /* no-autoconfig */
                    {
                        enable_autoconf = FALSE;
                        i++;
                    }
                    else
                    {
                        CLI_LIB_PrintStr_1("Unknown argument %s.\r\n", arg[i]);
                        return CLI_NO_ERROR;
                    }
                }
            }

            if(NETCFG_TYPE_OK != NETCFG_PMGR_ND_SetNdPrefix(ctrl_P->CMenu.vlan_ifindex,
                &prefix, valid_lifetime, preferred_lifetime, enable_onlink, enable_autoconf))
            {
                CLI_LIB_Printf("Failed to config router advertisement IPv6 prefixes\r\n");
            }

            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IPV6_ND_PREFIX:
            //ipv6-prefix
            if(arg[0] == NULL)
            {
                CLI_LIB_Printf("Unknown error!\r\n");
                break;
            }

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC,
                                                               arg[0],
                                                               (L_INET_Addr_T *)&prefix,
                                                               sizeof(prefix)))
            {
                CLI_LIB_Printf("Incorrect IPv6 address/prefix input!\r\n");
                break;
            }
            if(NETCFG_TYPE_OK != NETCFG_PMGR_ND_UnsetNdPrefix(ctrl_P->CMenu.vlan_ifindex, &prefix))
            {
                CLI_LIB_Printf("Failed to remove router advertisement IPv6 prefixes\r\n");
                break;
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 prefix: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif /*#if (SYS_CPNT_IPV6_ROUTING == TRUE)*/
    return (UI32_T)CLI_NO_ERROR;
}


UI32_T  CLI_API_IPV6_NsInterval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

   UI32_T value;

    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IPV6_ND_NSINTERVAL:

            //just value
            if(arg[0] == NULL)
            {
                CLI_LIB_Printf("Unknown error!\r\n");
                break;
            }
            value = strtoul(arg[0], (char **)NULL, 10);

            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdNsInterval(ctrl_P->CMenu.vlan_ifindex, value))
            {
                CLI_LIB_Printf("Failed to config IPv6 ns interval\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IPV6_ND_NSINTERVAL:
            //set default value
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_UnsetNdNsInterval(ctrl_P->CMenu.vlan_ifindex))
            {
                CLI_LIB_Printf("Failed to unset IPv6 ns interval\r\n");
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 : possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
    return (UI32_T)CLI_NO_ERROR;
}

UI32_T  CLI_API_IPV6_NdReachableTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6 == TRUE)
   UI32_T value;

    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IPV6_ND_REACHABLETIME:

            //just value
            if(arg[0] == NULL)
            {
                CLI_LIB_Printf("Unknown error!\r\n");
                break;
            }
            value = atoi(arg[0] );
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdReachableTime(ctrl_P->CMenu.vlan_ifindex, value))
            {
                CLI_LIB_Printf("Failed to config reachable time\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IPV6_ND_REACHABLETIME:
            //set default value
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_UnsetNdReachableTime(ctrl_P->CMenu.vlan_ifindex))
            {
                CLI_LIB_Printf("Failed to unset reachable time\r\n");
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 : possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif /*#if (SYS_CPNT_IPV6 == TRUE)*/

    return (UI32_T)CLI_NO_ERROR;
}
UI32_T CLI_API_IPV6_NdRouterLifeTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
   UI32_T value;
    UI32_T ret;
    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_IPV6_ND_RA_LIFETIME:
            //just value
            if(arg[0] == NULL)
            {
                CLI_LIB_Printf("Unknown error!\r\n");
                break;
            }
            value = atoi(arg[0] );
            ret = NETCFG_PMGR_ND_SetNdRaLifetime(ctrl_P->CMenu.vlan_ifindex, value);
            if(NETCFG_TYPE_OK!=ret)
            {
                if(NETCFG_TYPE_RA_LIFETIME_LESS_THAN_RA_INTERVAL == ret)
                    CLI_LIB_Printf("RA lifetime must not less than RA interval\r\n");
                else
                    CLI_LIB_Printf("Failed to set IPv6 router lifetime\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W5_NO_IPV6_ND_RA_LIFETIME:
            //set default value
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_UnsetNdRaLifetime(ctrl_P->CMenu.vlan_ifindex))
            {
                CLI_LIB_Printf("Failed to unset IPv6 router lifetime\r\n");
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 : possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif /*#if (SYS_CPNT_IPV6_ROUTING == TRUE)*/
    return (UI32_T)CLI_NO_ERROR;
}
UI32_T  CLI_API_IPV6_NdManagedConfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IPV6_ND_MANAGEDCONFIGFLAG:
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdManagedConfigFlag(ctrl_P->CMenu.vlan_ifindex, TRUE))
            {
                CLI_LIB_Printf("Failed to enable managed flag\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IPV6_ND_MANAGEDCONFIGFLAG:
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdManagedConfigFlag(ctrl_P->CMenu.vlan_ifindex, FALSE))
            {
                CLI_LIB_Printf("Failed to disable managed flag\r\n");
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 : possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif /*#if (SYS_CPNT_IPV6_ROUTING == TRUE)*/
    return (UI32_T)CLI_NO_ERROR;
}

UI32_T  CLI_API_IPV6_NdOtherConfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_ROUTING == TRUE)

    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IPV6_ND_OTHERCONFIGFLAG:
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdOtherConfigFlag(ctrl_P->CMenu.vlan_ifindex, TRUE))
            {
                CLI_LIB_Printf("Failed to enable other flag\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IPV6_ND_OTHERCONFIGFLAG:
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdOtherConfigFlag(ctrl_P->CMenu.vlan_ifindex, FALSE))
            {
                CLI_LIB_Printf("Failed to disable other flag\r\n");
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 : possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif /*#if (SYS_CPNT_IPV6_ROUTING == TRUE)*/
    return (UI32_T)CLI_NO_ERROR;
}








UI32_T  CLI_API_IPV6_NdRaSuppress(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
//    I32_T ret;
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_IPV6_ND_RA_SUPPRESS:
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdRaSuppress(ctrl_P->CMenu.vlan_ifindex, TRUE))
            {
                CLI_LIB_Printf("Failed to set suppress\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W5_NO_IPV6_ND_RA_SUPPRESS:
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdRaSuppress(ctrl_P->CMenu.vlan_ifindex, FALSE))
            {
                CLI_LIB_Printf("Failed to unset suppress\r\n");
            }
            break;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        case PRIVILEGE_CFG_INTERFACE_TUNNEL_CMD_W4_IPV6_ND_RA_SUPPRESS:
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdRaSuppress(ctrl_P->CMenu.tunnel_ifindex, TRUE))
            {
                CLI_LIB_Printf("Failed to set suppress\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_TUNNEL_CMD_W5_NO_IPV6_ND_RA_SUPPRESS:
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdRaSuppress(ctrl_P->CMenu.tunnel_ifindex, FALSE))
            {
                CLI_LIB_Printf("Failed to unset suppress\r\n");
            }
            break;
#endif /*SYS_CPNT_IP_TUNNEL*/
        default:
            CLI_LIB_Printf("(no) IPv6 : possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif /*#if (SYS_CPNT_IPV6_ROUTING == TRUE)*/
    return (UI32_T)CLI_NO_ERROR;
}

UI32_T  CLI_API_IPV6_NdRaInterval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    UI32_T min, max;

    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_IPV6_ND_RA_INTERVAL:
            if(arg[0] == NULL)
            {
                CLI_LIB_Printf("No argument error!\r\n");
                break;
            }
            max = atoi(arg[0]);
            if (arg[1] != NULL) /* rfc4861 page 41 */
            {
                min = atoi(arg[1]);
            }
            else
            {
                min = 0.33 * max;
                if (min < NETCFG_MGR_ND_MIN_RA_INTERVAL_RANGE_MIN)
                    min = NETCFG_MGR_ND_MIN_RA_INTERVAL_RANGE_MIN;
            }

            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdRaInterval(ctrl_P->CMenu.vlan_ifindex, max, min))
            {
                CLI_LIB_Printf("Failed to config IPv6 ra interval\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W5_NO_IPV6_ND_RA_INTERVAL:
            //set default value
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_UnsetNdRaInterval(ctrl_P->CMenu.vlan_ifindex))
            {
                CLI_LIB_Printf("Failed to unset IPv6 ra interval\r\n");
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 : possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif /*#if (SYS_CPNT_IPV6_ROUTING == TRUE)*/
    return (UI32_T)CLI_NO_ERROR;
}

UI32_T  CLI_API_IPV6_NdRouterPreference(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    /*L_INET_AddrIp_T    ipv6_addr;*/


    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_IPV6_ND_RA_ROUTERPREFERENCE:
            //ipv6-address+interface-type+interface-number+hardware-address
            if(arg[0] == NULL )
            {
                CLI_LIB_Printf("Unknown error!\r\n");
                break;
            }
            switch (arg[0][0]){
                case 'H':
                case 'h':
                    if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdRouterPreference(ctrl_P->CMenu.vlan_ifindex,NETCFG_TYPE_ND_ROUTER_PERFERENCE_HIGH))
                    {
                        CLI_LIB_Printf("Failed to set IPv6 router preference high\r\n");
                    }
                    break;
                case 'M':
                case 'm':
                    if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdRouterPreference(ctrl_P->CMenu.vlan_ifindex,NETCFG_TYPE_ND_ROUTER_PERFERENCE_MEDIUM))
                    {
                        CLI_LIB_Printf("Failed to set IPv6 router preference medium\r\n");
                    }
                    break;
                case 'L':
                case 'l':
                    if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdRouterPreference(ctrl_P->CMenu.vlan_ifindex,NETCFG_TYPE_ND_ROUTER_PERFERENCE_LOW))
                    {
                        CLI_LIB_Printf("Failed to set IPv6 router preference low\r\n");
                    }
                    break;
                default:
                    CLI_LIB_Printf("unknown preference value!\r\n");
                    break;
            }
            break;
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W5_NO_IPV6_ND_RA_ROUTERPREFERENCE:
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdRouterPreference(ctrl_P->CMenu.vlan_ifindex,SYS_DFLT_ND_ROUTER_ADVERTISEMENTS_DEFAULT_ROUTER_PREFERENCE))
            {
                CLI_LIB_Printf("Failed to unset IPv6 router preference \r\n");
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 neighbor: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
        }
#endif /*#if (SYS_CPNT_IPV6_ROUTING == TRUE)*/
        return (UI32_T)CLI_NO_ERROR;
}
/*End of IPV6*/
UI32_T  CLI_API_IPV6_NdHopLimit(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
/*    I32_T ret;*/
   UI32_T value;

    switch (cmd_idx){
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_IPV6_HOPLIMIT:

            //just value
            if(arg[0] == NULL)
            {
                CLI_LIB_Printf("Unknown error!\r\n");
                break;
            }
            value = atoi(arg[0] );
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdHoplimit( value))
            {
                CLI_LIB_Printf("Failed to config IPv6 hop limit\r\n");
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IPV6_HOPLIMIT:
            //set default value
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_UnsetNdHoplimit())
            {
                CLI_LIB_Printf("Failed to unset IPv6 ns interval\r\n");
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 : possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif /*#if (SYS_CPNT_IPV6_ROUTING == TRUE)*/
    return (UI32_T)CLI_NO_ERROR;
}

UI32_T  CLI_API_Ipv6_Nd_Dad_Attempts(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
/*    I32_T ret;*/
   UI32_T value;

    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_IPV6_ND_DAD_ATTEMPTS:

            //just value
            if(arg[0] == NULL)
            {
                CLI_LIB_Printf("Unknown error!\r\n");
                break;
            }
            value = atoi(arg[0] );
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdDADAttempts(ctrl_P->CMenu.vlan_ifindex, value))
            {
                CLI_LIB_Printf("Failed to config IPv6 DAD attempts\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W5_NO_IPV6_ND_DAD_ATTEMPTS:
            //set default value
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_SetNdDADAttempts(ctrl_P->CMenu.vlan_ifindex, SYS_DFLT_ND_DUPLICATE_ADDRESS_DETECTION_ATTEMPTS))
            {
                CLI_LIB_Printf("Failed to unset IPv6 DAD attempts\r\n");
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 : possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
    return (UI32_T)CLI_NO_ERROR;
}

UI32_T  CLI_API_Ipv6_Nd_Ns_Interval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
  return  CLI_API_IPV6_NsInterval( cmd_idx, arg, ctrl_P);
}

UI32_T  CLI_API_Ipv6_Neighbor(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    return CLI_API_IPV6_Neighbor(cmd_idx, arg, ctrl_P);
}

/* show ipv6 neighbor */
#if 0 /* output example */
State: I1 - Incomplete, I2 - Invalid, R - Reachable, S - Stale, D - Delay,
       P1 - Probe, P2 - Permanent, U - Unknown
IPv6 Address                            Age  Link-layer Addr   State Interface
--------------------------------------- ---- ----------------- ----- ---------
FE80::222:2FFF:FE84:2E00                  34 00-22-2F-84-2E-00 S     VLAN 3
FE80::250:BAFF:FE72:4B7C                   4 00-50-BA-72-4B-7C R     Craft
#endif
UI32_T CLI_API_Show_Ipv6_Neighbors(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    UI32_T vid = 0;
    UI32_T vid_ifindex = 0;
    L_INET_AddrIp_T  addr;
    char *NetToMediaState[] = {"","R","S","D","P1","I2","U","I1"};
    NETCFG_TYPE_IpNetToPhysicalEntry_T entry;
    char str_ip6[L_INET_MAX_IPADDR_STR_LEN+1] = {0};


    sprintf(buff, "State: I1 - Incomplete, I2 - Invalid, R - Reachable, S - Stale, D - Delay,\r\n");
	PROCESS_MORE(buff);
    sprintf(buff + strlen(buff),"       P1 - Probe, P2 - Permanent, U - Unknown\r\n");
	PROCESS_MORE(buff);
    sprintf(buff + strlen(buff), "%-39s %-4s %-17s %-5s %-9s\r\n", "IPv6 Address", "Age", "Link-layer Addr", "State", "Interface");
	PROCESS_MORE(buff);
    sprintf(buff + strlen(buff), "%-39s %-4s %-17s %-5s %-9s\r\n", "---------------------------------------", "----", "-----------------", "-----", "---------");
	PROCESS_MORE(buff);
    memset(&entry, 0, sizeof(entry));
    entry.ip_net_to_physical_net_address.type = L_INET_ADDR_TYPE_IPV6; /* must */

    /* show ipv6 neighbor*/
    if(NULL == arg[0])
    {

	    while(NETCFG_TYPE_OK == NETCFG_PMGR_ND_GetNextIpv6NetToPhysicalEntry(&entry))
	    {

	        L_INET_InaddrToString((L_INET_Addr_T *)&(entry.ip_net_to_physical_net_address), str_ip6, sizeof(str_ip6));
	        /* append to the same buffer*/
	        sprintf(buff + strlen(buff), "%-39s", str_ip6);

	        if (entry.ip_net_to_physical_type == VAL_ipNetToMediaType_static)
	        {
	            sprintf(buff + strlen(buff), " %4s", "----");            //age
	        }
	        else
	        {
	            sprintf(buff + strlen(buff), " %4lu", (unsigned long)entry.ip_net_to_physical_last_update);           //age
	        }
	        sprintf(buff + strlen(buff), " %02X-%02X-%02X-%02X-%02X-%02X", L_INET_EXPAND_MAC(entry.ip_net_to_physical_phys_address.phy_address_cctet_string));

            /* if entry type is static, we display P2(stands for Permanent) in "STATE" column,
             * because static entry won't change to other state */
            if (entry.ip_net_to_physical_type == VAL_ipNetToMediaType_static)
            {
                sprintf(buff + strlen(buff), " %-5s", "P2");
            }
            else
            {
                sprintf(buff + strlen(buff), " %-5s", (entry.ip_net_to_physical_state >7 || entry.ip_net_to_physical_state <1) ? "UNKNO":NetToMediaState[entry.ip_net_to_physical_state]);
            }
#if (SYS_CPNT_CRAFT_PORT == TRUE)
            if(SYS_ADPT_CRAFT_INTERFACE_IFINDEX == entry.ip_net_to_physical_if_index)
            {
                sprintf(buff + strlen(buff), " Craft\r\n");
            }
            else
#endif
            {
                VLAN_OM_ConvertFromIfindex(entry.ip_net_to_physical_if_index, &vid);
                sprintf(buff + strlen(buff), " VLAN %-4lu\r\n", (unsigned long)vid);
            }

	        PROCESS_MORE(buff);
	    }

    }
    else
    {
        /* show ipv6 neighbor vlan x*/
        if((arg[0][0] == 'v')||(arg[0][0] == 'V'))
        {
            vid = CLI_LIB_AtoUl(arg[1],10);

            VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
	    	while(NETCFG_TYPE_OK == NETCFG_PMGR_ND_GetNextIpv6NetToPhysicalEntry(&entry))
	    	{

	           if(entry.ip_net_to_physical_if_index == vid_ifindex)
	           {
	        		L_INET_InaddrToString((L_INET_Addr_T *)&(entry.ip_net_to_physical_net_address), str_ip6, sizeof(str_ip6));
	        		/* append to the same buffer*/
	        		sprintf(buff + strlen(buff), "%-40s", str_ip6);

	        		if (entry.ip_net_to_physical_type == VAL_ipNetToMediaType_static)
	        		{
	            		sprintf(buff + strlen(buff), "%-10s", "--");            //age
	        		}
	        		else
	        		{
	            		sprintf(buff + strlen(buff), "%-10lu", (unsigned long)entry.ip_net_to_physical_last_update);           //age
	        		}
	        		sprintf(buff + strlen(buff), "%02X-%02X-%02X-%02X-%02X-%02X ", L_INET_EXPAND_MAC(entry.ip_net_to_physical_phys_address.phy_address_cctet_string));
	        		/* if entry type is static, we display static in "STATE" column,
                     * because static entry won't change to other state */
                    if (entry.ip_net_to_physical_type == VAL_ipNetToMediaType_static)
                    {
                        sprintf(buff + strlen(buff), "%5s ", "P2");
                    }
                    else
                    {
                        sprintf(buff + strlen(buff), "%5s ", (entry.ip_net_to_physical_state >7 || entry.ip_net_to_physical_state <1) ? "UNKNO":NetToMediaState[entry.ip_net_to_physical_state]);
                    }

	        		if(VLAN_OM_ConvertFromIfindex(entry.ip_net_to_physical_if_index, &vid))
	        		{
	            		sprintf(buff + strlen(buff), "%4lu ",(unsigned long)vid);
	        		}
	        		sprintf(buff + strlen(buff), "\r\n");
	        		PROCESS_MORE(buff);
	    		}

	      	}

        }
        else
        {
            memset(&addr, 0 , sizeof(L_INET_AddrIp_T));
            L_INET_StringToInaddr(L_INET_FORMAT_IPV6_ADDR_RAW, arg[0], (L_INET_Addr_T *) &addr, sizeof(addr));

            /* show ipv6 neighbor xx:xx:xx:xx..*/
            while(NETCFG_TYPE_OK == NETCFG_PMGR_ND_GetNextIpv6NetToPhysicalEntry(&entry))
            {
                if(0 == memcmp(&(entry.ip_net_to_physical_net_address.addr), &addr.addr, SYS_ADPT_IPV6_ADDR_LEN))
                {
                    L_INET_InaddrToString((L_INET_Addr_T *)&(entry.ip_net_to_physical_net_address), str_ip6, sizeof(str_ip6));
                    /* append to the same buffer*/
                    sprintf(buff + strlen(buff), "%-40s", str_ip6);

                    if (entry.ip_net_to_physical_type == VAL_ipNetToMediaType_static)
                    {
                        sprintf(buff + strlen(buff), "%-10s", "--");            //age
                    }
                    else
                    {
                        sprintf(buff + strlen(buff), "%-10lu", (unsigned long)entry.ip_net_to_physical_last_update);           //age
                    }
                    sprintf(buff + strlen(buff), "%02X-%02X-%02X-%02X-%02X-%02X ", L_INET_EXPAND_MAC(entry.ip_net_to_physical_phys_address.phy_address_cctet_string));
                    /* if entry type is static, we display static in "STATE" column,
                     * because static entry won't change to other state */
                    if (entry.ip_net_to_physical_type == VAL_ipNetToMediaType_static)
                    {
                        sprintf(buff + strlen(buff), "%5s ", "P2");
                    }
                    else
                    {
                        sprintf(buff + strlen(buff), "%5s ", (entry.ip_net_to_physical_state >7 || entry.ip_net_to_physical_state <1) ? "UNKNO":NetToMediaState[entry.ip_net_to_physical_state]);
                    }

                    if(VLAN_OM_ConvertFromIfindex(entry.ip_net_to_physical_if_index, &vid))
                    {
                        sprintf(buff + strlen(buff), "%4lu ",(unsigned long)vid);
                    }
                    sprintf(buff + strlen(buff), "\r\n");
                    PROCESS_MORE(buff);
                }

            }

        }
    }

    return (UI32_T)CLI_NO_ERROR;
}

UI32_T  CLI_API_Clear_Ipv6_Neighbors(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
/*    I32_T ret;*/
    switch (cmd_idx){
        case PRIVILEGE_EXEC_CMD_W3_CLEAR_IPV6_NEIGHBORS:
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ND_DeleteAllDynamicIpv6NetToMediaEntry())
            {
                CLI_LIB_Printf("Failed to clear IPv6 neighbor cache.\r\n");
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 : possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
    return (UI32_T)CLI_NO_ERROR;
}

/* FUNTION NAME: CLI_API_Ipv6_Enable
 * PURPOSE:
 *      Enable or disable ipv6 process on the ipv6 interface.
 *      The common API of both commands:
 *          ipv6 enable
 *          no ipv6 enable
 *
 * INPUT:
 *      cmd_idx     -- command index
 *      arg         -- available addtional parameters array
 *      ctrl_P      -- CLI work space structure
 *
 * OUTPUT:
 *      None.
 * NOTES:
 *      None.
 */
UI32_T CLI_API_Ipv6_Enable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

#if (SYS_CPNT_IPV6 == TRUE)
    UI32_T cli_ifindex;
    UI32_T rc_netcfg;

    /* DEBUG */
    //printf("%s, %d, %s\n", __FUNCTION__, __LINE__, "");

    //VLAN_OM_ConvertToIfindex(ctrl_P->CMenu.vlan_ifindex, &cli_ifindex);
    cli_ifindex = ctrl_P->CMenu.vlan_ifindex;
    /* DEBUG */
    //printf("%s, %d, %d\n", __FUNCTION__, __LINE__, cli_ifindex);

    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_IPV6_ENABLE :
            /* Basen 2005.6.20, Update management VLAN for IPv6 */
            // peter_yu, IP6_MGR_UpdateManageVlan(ctrl_P->CMenu.vlan_ifindex)!= IPV6_MGR_SUCCESS )
            //    CLI_LIB_Printf("Enable ipv6 process on interface %ld failed!\r\n", cli_ifindex);

            if(NETCFG_PMGR_IP_IPv6Enable(cli_ifindex) != NETCFG_TYPE_OK)
                CLI_LIB_Printf("Failed to enable IPv6 processing.\r\n", cli_ifindex);
            break;
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_NO_IPV6_ENABLE :
            rc_netcfg = NETCFG_PMGR_IP_IPv6Disable(cli_ifindex);
            switch (rc_netcfg)
            {
                case NETCFG_TYPE_OK:
                    /* succeed */
                    break;
                case NETCFG_TYPE_HAS_EXPLICIT_IPV6_ADDR:
                    CLI_LIB_PrintStr("Failed to disable IPv6 processing because there is an explicit IPv6 address.\r\n");
                    break;
                default:
    /* DEBUG */
    /*printf("%s, %d, rc: 0x%lx\n", __FUNCTION__, __LINE__, rc_netcfg);*/

                    CLI_LIB_PrintStr("Failed to disable IPv6 processing.");
                    break;
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 enable: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif /* SYS_CPNT_IPV6 */
    return (UI32_T)CLI_NO_ERROR;
}


/* FUNTION NAME: CLI_API_Ipv6_Address
 * PURPOSE:
 *      Add ipv6 address to ipv6 interface.
 *      The common API of the following CLI commands:
 *          ipv6 address ipv6-address [ link-local]
 *          ipv6 address ipv6-prefix [eui-64]
 *
 * INPUT:
 *      cmd_idx     -- command index
 *      arg         -- available addtional parameters array
 *      ctrl_P      -- CLI work space structure
 *
 * OUTPUT:
 *      None.
 * NOTES:
 *      None.
 */
UI32_T CLI_API_Ipv6_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6 == TRUE)

    UI32_T cli_ifindex;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    UI32_T ipv6_addr_type;
    UI32_T rc_netcfg;

    cli_ifindex = ctrl_P->CMenu.vlan_ifindex;

    ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_IPV6_ADDRESS:

            memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_ADDR_RAW,
                                                               arg[0],
                                                               (L_INET_Addr_T *)&rif_config.addr,
                                                               sizeof(rif_config.addr)))
            {
                CLI_LIB_PrintStr("Invalid IPv6 address format.\r\n");
                return CLI_NO_ERROR;
            }
            /* use fe80::/10 to check link-local
               use fe80::/64 to check valid link-local
             */
            if(arg[1] == NULL)
            {
                /* check link-local address */
                if(L_INET_ADDR_IS_IPV6_LINK_LOCAL(rif_config.addr.addr))
                {
                    CLI_LIB_PrintStr("Please use: ""IPv6 address X:X:X:X::X link-local"" instead \r\n");
                    return CLI_NO_ERROR;
                }
                        
                ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL;
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
                VLAN_OM_ConvertFromIfindex(cli_ifindex, &rif_config.addr.zoneid);

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
                return CLI_NO_ERROR;
            }

            rif_config.ifindex = cli_ifindex;
            rif_config.ipv6_addr_config_type = NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_MANUAL;
            rif_config.ipv6_addr_type = ipv6_addr_type;
            rif_config.row_status = VAL_netConfigStatus_2_createAndGo;

            if(rif_config.ipv6_addr_type == NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL)
                rif_config.addr.preflen = 64;

            rc_netcfg = NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
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
                case NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED:
                    CLI_LIB_PrintStr("Failed to add IPv6 address. Overlapped with existing address.\r\n");
                    return CLI_NO_ERROR;
                default:
                    /* unknown error */
                    CLI_LIB_PrintStr("Failed to add IPv6 address.\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_NO_IPV6_ADDRESS:
#if 0 // peter_yu, not support
            if(arg[0] == NULL)
            {
                if(IP6_MGR_DelAllManualAddresses(cli_ifindex) != IPV6_MGR_SUCCESS)
                {
                    CLI_LIB_PrintStr("Delete all manually configurated IPv6 address in this interface failed.\r\n");
                }
            }
#endif
            memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_ADDR_RAW,
                                                               arg[0],
                                                               (L_INET_Addr_T *)&rif_config.addr,
                                                               sizeof(rif_config.addr)))
            {
                CLI_LIB_PrintStr("Invalid IPv6 address format.\r\n");
                return CLI_NO_ERROR;
            }

            if(arg[1] == NULL)
            {
                /* check link-local address */
                if(L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL(rif_config.addr.addr))
                {
                    CLI_LIB_PrintStr("Please use: ""no IPv6 address X:X:X:X::X link-local"" instead \r\n");
                    return CLI_NO_ERROR;
                }
                ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL;
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
                VLAN_OM_ConvertFromIfindex(cli_ifindex, &rif_config.addr.zoneid);

            }
            else if((arg[1][0] == 'e') || (arg[1][0] == 'E')) /* eui-64 */
            {
                ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_EUI64;
            }
            else
            {
                CLI_LIB_PrintStr("Invalid IPv6 address format.\r\n");
                return CLI_NO_ERROR;
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
                case NETCFG_TYPE_OK:
                    return CLI_NO_ERROR;
                case NETCFG_TYPE_FAIL:
                default:
                    CLI_LIB_PrintStr("Failed to delete IPv6 address.\r\n");
                    return CLI_NO_ERROR;
            }
            break;

        default:
            CLI_LIB_PrintStr_1("(no) IPv6 address: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif /* SYS_CPNT_IPV6 */
    return (UI32_T)CLI_NO_ERROR;

}

/* FUNTION NAME: CLI_API_Ipv6_Address_Autoconfig
 * PURPOSE:
 *      Enable/Disable IPv6 address automatic configuration
 *      The common API of two commands:
 *          IPv6 address autoconfig
 *          no IPv6 address autoconfig
 *
 * INPUT:
 *      cmd_idx     -- command index
 *      arg         -- available addtional parameters array
 *      ctrl_P      -- CLI work space structure
 *
 * OUTPUT:
 *      None.
 * NOTES:
 *      None.
 */
UI32_T  CLI_API_Ipv6_Address_Autoconfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ((SYS_CPNT_IPV6 == TRUE)&&(SYS_CPNT_IPV6_ROUTING == FALSE))

    UI32_T cli_ifindex;

    cli_ifindex = ctrl_P->CMenu.vlan_ifindex;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IPV6_ADDRESS_AUTOCONFIG:

            /* Basen 2005.6.20, Update management VLAN for IPv6 */
            // peter_yu, if(IP6_MGR_UpdateManageVlan(ctrl_P->CMenu.vlan_ifindex)!= IPV6_MGR_SUCCESS )
            //    CLI_LIB_PrintStr("Enable address automatic configuration failed!\r\n");

            if(FALSE == CMGR_SetIpv6AddrAutoconfig(cli_ifindex, TRUE))
            {
                CLI_LIB_PrintStr("Failed to enable IPv6 address auto configuration.\r\n");
            }
            break;
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IPV6_ADDRESS_AUTOCONFIG:
            if(FALSE == CMGR_SetIpv6AddrAutoconfig(cli_ifindex, FALSE))
            {
                CLI_LIB_PrintStr("Failed to enable IPv6 address auto configuration.\r\n");
            }
            break;
        default:
            CLI_LIB_PrintStr_1("(no) IPv6 address autoconfig: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }

#endif /* SYS_CPNT_IPV6 */

    return (UI32_T)CLI_NO_ERROR;
}

/* FUNTION NAME: CLI_API_Ipv6_Mtu
 * PURPOSE:
 *      Set or unset the IPv6 interface MTU.
 *      The common API of both commands:
 *          IPv6 mtu <1280-65535>
 *          no IPv6 mtu
 *
 * INPUT:
 *      cmd_idx     -- command index
 *      arg         -- available addtional parameters array
 *      ctrl_P      -- CLI work space structure
 *
 * OUTPUT:
 *      None.
 * NOTES:
 *      None.
 */
UI32_T  CLI_API_Ipv6_Mtu(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6 == TRUE)
    UI32_T cli_ifindex;
    NETCFG_TYPE_L3_Interface_T intf;

    cli_ifindex = ctrl_P->CMenu.vlan_ifindex;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_IPV6_MTU:
            if(arg[0] !=NULL && arg[1] == NULL)
            {
                UI32_T cli_if_mtu = atoi(arg[0]);
                UI32_T rc;

                if((rc = NETCFG_PMGR_IP_SetIPv6InterfaceMTU(cli_ifindex, cli_if_mtu)) != NETCFG_TYPE_OK)
                {
                    if (rc == NETCFG_TYPE_IPV6_MTU_EXCEED_IF_MTU)
                    {
                        intf.ifindex = cli_ifindex;
                        if (NETCFG_TYPE_OK == NETCFG_POM_IP_GetL3Interface(&intf))
                            CLI_LIB_PrintStr_1("Invalid IPv6 MTU. Can't exceed MTU of interface (%lu bytes).\r\n",(unsigned long)intf.u.physical_intf.mtu)
                        else
                            CLI_LIB_PrintStr("Invalid IPv6 MTU. Can't exceed MTU of interface.\r\n");
                    }
                    else
                    {
                        CLI_LIB_PrintStr("Failed to set IPv6 MTU.\r\n");
                    }
                }
            }
            break;
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_NO_IPV6_MTU:
            if(NETCFG_PMGR_IP_UnsetIPv6InterfaceMTU(cli_ifindex) != NETCFG_TYPE_OK)
            {
                 CLI_LIB_PrintStr("Failed to unset IPv6 MTU.\r\n");
            }
            break;

        default:
            CLI_LIB_PrintStr_1("(no) IPv6 mtu: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif /* SYS_CPNT_IPV6 */
    return (UI32_T)CLI_NO_ERROR;
}




/* FUNTION NAME: CLI_API_Show_Ipv6_Interface
 * PURPOSE:
 *      Display the information of IPv6 interface
 *      The API of command "show IPv6 interface"
 *
 * INPUT:
 *      cmd_idx     -- command index
 *      arg         -- available addtional parameters array
 *      ctrl_P      -- CLI work space structure
 *
 * OUTPUT:
 *      None.
 * NOTES:
 *      None.
 */
#if 0 /* brief mode output example */
Interface       Status     IPv6       IPv6 Address
--------------- ---------- ---------- ---------------------------------------
VLAN 1          Up         Up         FE80::768E:F8FF:FE68:870
VLAN 1          Up         Up         2001:1DB8:1111:2F3B:12AA:11FF:FE28:9C5A
VLAN 2          Up         Down       Unassigned
Craft           Up         Up         FE80::768E:F8FF:FE68:870
#endif
UI32_T  CLI_API_Show_Ipv6_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6 == TRUE)

    /* L3 interface */
    NETCFG_TYPE_L3_Interface_T l3_intf;

    /* Rif info */

    NETCFG_TYPE_InetRifConfig_T rif_config;

    char                    addr_buf[L_INET_MAX_IPADDR_STR_LEN+1] = {0};/* should larger than L_INET_MAX_IPADDR_STR_LEN (54) */
    char                    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char                    *op_status[3]={"up","down","not existing"}; /* Vlan status added. Basen 2005.5.17 */
    int                     op_index = 1;   /*  down    */
    UI32_T                  vlan_id = 0; /* specified vlan ifindex */
    UI32_T                  line_num = 0;
    L_INET_AddrIp_T         maddr;  /* multicast address */
    UI32_T                  num_linklocal_addr, num_global_addr, num_maddr, ipv6_mtu;
    NETCFG_TYPE_IpAddressInfoEntry_T addr_info;

    /* vlan info */
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;

    /* ND info */
    UI32_T                  if_dad_attempts; /* , if_reachable_time; */ /* 2005-04-28, stj del -, unused variable */
    UI32_T                  if_retransmit_interval;  /* Ed_huang 2005.10.27 added, show ND retransmit interval */
    UI32_T                  if_reachable_time;
    UI32_T                  ra_lifetime;
    /* options variable */
    UI32_T  vid;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    UI32_T tid;
#endif /*SYS_CPNT_IP_TUNNEL*/
    BOOL_T  brief = FALSE;
    char   str_vlan_status[5] = {0};
    char   str_ipv6_status[5] = {0};
    char   str_addr_status[20] = {0};
    char   str_ipv6_prefix[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
    BOOL_T  is_config_set = FALSE;
    /* BODY
     */
    num_linklocal_addr = num_global_addr = num_maddr = 0;

    if(arg[0] != NULL && (arg[0][0] == 'B' || arg[0][0] == 'b'))
        brief = TRUE;

    if(arg[1] != NULL && arg[2] !=NULL)
    {
        if(arg[1][0] == 'V' || arg[1][0] == 'v')
        {
            vlan_id = atoi(arg[2]);
        }
        else
        {

            sprintf(buff + strlen(buff),"show IPv6 interface: incorrect parameter %s\r\n", arg[0]);
            PROCESS_MORE(buff);
            return (UI32_T)CLI_NO_ERROR;
        }
    }

    if (brief)
    {

        sprintf(buff + strlen(buff),
        "Interface      "  /* 15 */
        " "
        "Status    "       /* 10 */
        " "
        "IPv6      "       /* 10 */
        " "
        "IPv6 Address\r\n"     /* 39 */);
        sprintf(buff + strlen(buff),"%.15s","-----------------");
        sprintf(buff + strlen(buff)," ");
        sprintf(buff + strlen(buff),"%.10s","-----------------");
        sprintf(buff + strlen(buff)," ");
        sprintf(buff + strlen(buff),"%.10s","-----------------");
        sprintf(buff + strlen(buff)," ");
        sprintf(buff + strlen(buff),"%.39s\r\n","---------------------------------------");
        PROCESS_MORE(buff);
    }

    memset(&l3_intf, 0, sizeof(l3_intf));
    /* not specified vlan ifindex*/
    if(vlan_id == 0)
    {
	    while(NETCFG_POM_IP_GetNextL3Interface(&l3_intf) == NETCFG_TYPE_OK)
	    {
#if (SYS_CPNT_IP_TUNNEL == TRUE)
	        if(IS_TUNNEL_IFINDEX(l3_intf.ifindex))
	        {
	            IP_LIB_ConvertTunnelIdFromIfindex(l3_intf.ifindex, &tid);
	            op_index = 1;   /*  down    */
	            INFOprintf("Get row_status= %d", l3_intf.u.tunnel_intf.row_status);
	            if(l3_intf.u.tunnel_intf.row_status == VAL_netConfigStatus_2_active)
	            {
	                op_index = 0;   /*  up  */
	            }
	        }
	        else
#endif /*SYS_CPNT_IP_TUNNEL*/
	        {
		        if (!VLAN_OM_ConvertFromIfindex(l3_intf.ifindex, &vid))
		            continue;

		        /* Vlan up/down info. */
		       if (VLAN_POM_GetDot1qVlanCurrentEntry(0,vid, &vlan_entry)==FALSE)
		            op_index = 2;   /*  none    */
		       else
		       {
		            if (vlan_entry.if_entry.vlan_operation_status == VAL_ifOperStatus_up )
		                op_index = 0;   /*  up  */
		            else
		                op_index = 1;   /*  down    */
		       }
        	}
        /* Show IPv6 address Info */
        if (!brief)
        {
            /* Show Vlan Info */
#if (SYS_CPNT_IP_TUNNEL == TRUE)
            if(IS_TUNNEL_IFINDEX(l3_intf.ifindex))
            {

                sprintf(buff + strlen(buff), "Tunnel %ld is %s\r\n", (long)tid,op_status[op_index]);
                PROCESS_MORE(buff);
            }
            else
#endif /*SYS_CPNT_IP_TUNNEL*/

            sprintf(buff + strlen(buff), "VLAN %ld is %s\r\n",  (long)vid,op_status[op_index]);
            PROCESS_MORE(buff);
            sprintf(buff + strlen(buff), "IPv6 is %s.\r\n", (l3_intf.ipv6_enable)? "enabled":"stale");
            PROCESS_MORE(buff);
            /* Link-local address */
                sprintf(buff + strlen(buff),"Link-local address:\r\n");
                PROCESS_MORE(buff);
            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.ifindex = l3_intf.ifindex;
            rif_config.addr.type = L_INET_ADDR_TYPE_IPV6Z;
            rif_config.addr.addr[0] = 0xFE;
            rif_config.addr.addr[1] = 0x80;
            rif_config.addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
            while(NETCFG_POM_IP_GetNextInetRifOfInterface(&rif_config) == NETCFG_TYPE_OK)
            {
                /*initial string*/
                memset(str_addr_status, 0, sizeof(str_addr_status));
                if(!L_INET_ADDR_IS_IPV6_LINK_LOCAL_UNICAST(rif_config.addr.addr))
                    break;
                num_linklocal_addr++;
                memset(addr_buf, 0, sizeof(addr_buf));
                L_INET_InaddrToString((L_INET_Addr_T *)&rif_config.addr, addr_buf, sizeof(addr_buf));
                sprintf(buff + strlen(buff),"  %s", addr_buf);

                /* Address state, spec required states: TEN, DUP, EUI.*/
                memset(&addr_info, 0, sizeof(addr_info));
                addr_info.ifindex = l3_intf.ifindex;
                memcpy(&addr_info.ipaddr, &rif_config.addr, sizeof(addr_info.ipaddr));
                if(NETCFG_TYPE_OK == NETCFG_PMGR_IP_GetIfIpv6AddrInfo(&addr_info))
                {
                    if(addr_info.tentative)
                        sprintf(str_addr_status, "%s", "TEN");
                }
                /* show invalid status */
                if(rif_config.flags & NETCFG_TYPE_RIF_FLAG_IPV6_INVALID)
                {
                    if(str_addr_status[0])
                        sprintf(str_addr_status, "%s", "/");
                    sprintf(str_addr_status, "%s", "INVALID");
                }
                if(str_addr_status[0])
                    sprintf(buff + strlen(buff),"[%s]", str_addr_status);

                sprintf(buff + strlen(buff), "\r\n");
                PROCESS_MORE(buff);
            }
            if(!num_linklocal_addr)
            {
                sprintf(buff + strlen(buff),"(None)\r\n");
                PROCESS_MORE(buff);
            }
            /* Global unicast address(es) */
            sprintf(buff + strlen(buff),"Global unicast address(es):\r\n");
            PROCESS_MORE(buff);
            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.ifindex = l3_intf.ifindex;
            rif_config.addr.type = L_INET_ADDR_TYPE_IPV6;
            while(NETCFG_POM_IP_GetNextInetRifOfInterface(&rif_config) == NETCFG_TYPE_OK)
            {
                /* initial string */
                memset(str_ipv6_prefix, 0, sizeof(str_ipv6_prefix));
                memset(str_addr_status, 0, sizeof(str_addr_status));

                if(L_INET_ADDR_IS_IPV6_LINK_LOCAL(rif_config.addr.addr)
                    || IP_LIB_IsIPv6Multicast(rif_config.addr.addr)
                    || IP_LIB_IsIPv6LoopbackAddr(rif_config.addr.addr))
                    break;
                num_global_addr++;

                L_INET_InaddrToString((L_INET_Addr_T *)&rif_config.addr, addr_buf, sizeof(addr_buf));
                NETCFG_TYPE_InetRifConfig_T rif_tmp;
                memcpy(&rif_tmp, &rif_config, sizeof(rif_tmp));



                if ( TRUE == IP_LIB_GetPrefixAddr(rif_config.addr.addr, rif_config.addr.addrlen, rif_config.addr.preflen, rif_tmp.addr.addr))
                {
                    /* ipv6 perfix address*/
                    L_INET_InaddrToString((L_INET_Addr_T *)&rif_tmp.addr, str_ipv6_prefix, sizeof(str_ipv6_prefix));
                }


                sprintf(buff + strlen(buff),"  %s, subnet is %s", addr_buf, str_ipv6_prefix);
                /* EUI-64 */
                if(rif_config.ipv6_addr_type == NETCFG_TYPE_IPV6_ADDRESS_TYPE_EUI64)
                {
                    sprintf(str_addr_status, "%s", "EUI");

                }

                /* Address state, spec required states: TEN, DUP, EUI.*/
                memset(&addr_info, 0, sizeof(addr_info));
                addr_info.ifindex = l3_intf.ifindex;
                memcpy(&addr_info.ipaddr, &rif_config.addr, sizeof(addr_info.ipaddr));
                if(NETCFG_TYPE_OK == NETCFG_PMGR_IP_GetIfIpv6AddrInfo(&addr_info))
                {
                    if(addr_info.tentative)
                    {
                        //if(!memcmp(str_addr_status, "", 1))
                        if(str_addr_status[0])
                            sprintf(str_addr_status, "%s", "/");
                        sprintf(str_addr_status, "%s", "TEN");
                    }
                }

                 /* show invalid status */
                if(rif_config.flags & NETCFG_TYPE_RIF_FLAG_IPV6_INVALID)
                {
                    if(str_addr_status[0])
                        sprintf(str_addr_status, "%s", "/");
                    sprintf(str_addr_status, "%s", "INVALID");
                }
                /* Autoconfig flag , valid and preferred lifetime
                 */
                if((rif_config.ipv6_addr_config_type == NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_STATEFULL)
                    || (rif_config.ipv6_addr_config_type == NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_STATELESS))
                {

                    if(str_addr_status[0])
                        sprintf(str_addr_status, "%s", "/");
                    sprintf(str_addr_status, "%s", "AUTOCONFIG");
	                sprintf(buff + strlen(buff), "[%s]\r\n", str_addr_status);
	                PROCESS_MORE(buff);
	                sprintf(buff + strlen(buff), "    valid lifetime %ld preferred lifetime %ld\r\n",(long)addr_info.valid_lft, (long)addr_info.preferred_lft);
	                PROCESS_MORE(buff);
                }
                else if (str_addr_status[0]) /* str_addr_status isn't empty */
                {

                    sprintf(buff + strlen(buff), "[%s]\r\n", str_addr_status);
                    PROCESS_MORE(buff);
                }
                else  /* str_addr_status is empty */
                {

                    sprintf(buff + strlen(buff), "\r\n");
                    PROCESS_MORE(buff);
                }
            }
            if(!num_global_addr)
            {

                sprintf(buff + strlen(buff), "(None)\r\n");
                PROCESS_MORE(buff);
            }
            /* Joined group address(es) */

            sprintf(buff + strlen(buff), "Joined group address(es):\r\n");
            PROCESS_MORE(buff);
            memset(&maddr, 0, sizeof(maddr));
            while(NETCFG_TYPE_OK == NETCFG_PMGR_IP_GetNextIfJoinIpv6McastAddr(l3_intf.ifindex, &maddr))
            {
                num_maddr++;
                memset(addr_buf, 0, sizeof(addr_buf));
                L_INET_InaddrToString((L_INET_Addr_T *)&maddr, addr_buf, sizeof(addr_buf));

                sprintf(buff + strlen(buff),"%s\r\n",addr_buf);
                PROCESS_MORE(buff);
            }
            if(!num_maddr)
            {

                sprintf(buff + strlen(buff), "(None)\r\n");
                PROCESS_MORE(buff);
            }
            /* interface MTU */
            if(NETCFG_POM_IP_GetIPv6InterfaceMTU(l3_intf.ifindex, &ipv6_mtu) == NETCFG_TYPE_OK)
            {

                sprintf(buff + strlen(buff), "IPv6 link MTU is %ld bytes\r\n", (long)ipv6_mtu);
                PROCESS_MORE(buff);
            }

            /* Show MTU, DAD attempts, ND retrans interval */

            if(NETCFG_POM_ND_GetNdDADAttempts(l3_intf.ifindex ,&if_dad_attempts) == TRUE)
            {
                if(if_dad_attempts == 0)
                {

                    sprintf(buff + strlen(buff), "ND DAD is disabled.\r\n");
                    PROCESS_MORE(buff);
                }
                else
                {

                    sprintf(buff + strlen(buff), "ND DAD is enabled, number of DAD attempts: %ld.\r\n", (long)if_dad_attempts);
                    PROCESS_MORE(buff);
                }
            }

            if(NETCFG_POM_ND_GetNdNsInterval(l3_intf.ifindex, &if_retransmit_interval) == TRUE)
            {

	                sprintf(buff + strlen(buff), "ND retransmit interval is %lu milliseconds\r\n",(unsigned long)if_retransmit_interval);
	                PROCESS_MORE(buff);
            }

            if(TRUE == NETCFG_POM_ND_IsConfigFlagSet(l3_intf.ifindex, NETCFG_OM_ND_FLAG_ISSET_NS_INTERVAL, &is_config_set))
            {
                UI32_T advert_value;
                if(is_config_set)
                    advert_value = if_retransmit_interval;
                else
                    advert_value = 0;
                sprintf(buff, "ND advertised retransmit interval is %ld milliseconds\r\n", (long)advert_value);
                PROCESS_MORE(buff);
            }

            /* nd reachable time */
            if(NETCFG_POM_ND_GetNdReachableTime(l3_intf.ifindex, &if_reachable_time) == TRUE)
            {
                sprintf(buff, "ND reachable time is %lu milliseconds\r\n", (unsigned long)if_reachable_time);
                PROCESS_MORE(buff);
            }

            if(TRUE == NETCFG_POM_ND_IsConfigFlagSet(l3_intf.ifindex, NETCFG_OM_ND_FLAG_ISSET_REACHABLE_TIME, &is_config_set))
            {
                UI32_T advert_value;
                if(is_config_set)
                    advert_value = if_reachable_time;
                else
                    advert_value = 0;
                sprintf(buff, "ND advertised reachable time is %ld milliseconds\r\n", (long)advert_value);
                PROCESS_MORE(buff);
            }

            if(NETCFG_POM_ND_GetNdRaLifetime(l3_intf.ifindex, &ra_lifetime) == TRUE)
            {
                sprintf(buff, "ND advertised router lifetime is %lu seconds\r\n", (unsigned long)ra_lifetime);
                PROCESS_MORE(buff);
            }

            sprintf(buff + strlen(buff), "\r\n");
            PROCESS_MORE(buff);

        }
        else
        {
            /* show ipv6 interface brief mode */
            int num_ipv6_addr = 0;


            memset(str_vlan_status, 0, sizeof(str_vlan_status));
            memset(str_ipv6_status, 0, sizeof(str_ipv6_status));

            /* Vlan Up/Down */
            switch (op_index)
            {
                case 0:
                    sprintf(str_vlan_status, "%s", "Up");
                    break;
                case 1:
                case 2:
                default:
                    sprintf(str_vlan_status, "%s", "Down");
                    break;
            }

            /* IPv6 Up/Down */
            sprintf(str_ipv6_status, "%s", (l3_intf.ipv6_enable)? "Up":"Down");

            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.addr.type = L_INET_ADDR_TYPE_IPV6;
            rif_config.ifindex = l3_intf.ifindex;
            while(NETCFG_POM_IP_GetNextInetRifOfInterface(&rif_config) == NETCFG_TYPE_OK)
            {
                char *char_p;

                if((rif_config.addr.type != L_INET_ADDR_TYPE_IPV6)
                    && (rif_config.addr.type != L_INET_ADDR_TYPE_IPV6Z))
                    break;
                num_ipv6_addr ++;

                /* IPv6 addresses */
                L_INET_InaddrToString((L_INET_Addr_T *)&rif_config.addr, addr_buf, sizeof(addr_buf));

                /* truncate zone-id(%N) for link-local address */
                char_p = strchr((char *)addr_buf, '%');
                if(char_p)
                {
                    /* link-local address */
                    *char_p = '\0';
                    sprintf((char *)addr_buf + strlen((char *)addr_buf), "/%d", rif_config.addr.preflen);
                }

#if (SYS_CPNT_IP_TUNNEL == TRUE)
                if(IS_TUNNEL_IFINDEX(rif_config.ifindex))
                {
                    sprintf(buff + strlen(buff),"TUNNEL %-8ld",tid);
                }
                else
#endif
                sprintf(buff + strlen(buff),"VLAN %-10ld",(unsigned long)vid);

                    sprintf(buff + strlen(buff),
                    " "
                    "%-10s"
                    " "
                    "%-10s"
                    " "
                    "%s\r\n"
                    ,
                    str_vlan_status,/* Vlan status */
                    str_ipv6_status,/* IPv6 status */
                    addr_buf        /* IPv6 addresses */
                    );
#if 0
                    sprintf(buff + strlen(buff),
                    "VLAN %-10ld"
                    " "
                    "%-10s"
                    " "
                    "%-10s"
                    " "
                    "%s\r\n"
                    ,
                    vid,            /* Vlan */
                    str_vlan_status,/* Vlan status */
                    str_ipv6_status,/* IPv6 status */
                    addr_buf        /* IPv6 addresses */
                    );
#endif
                    PROCESS_MORE(buff);
                } /* while */

            if(!num_ipv6_addr)
            {

#if (SYS_CPNT_IP_TUNNEL == TRUE)
                    if(IS_TUNNEL_IFINDEX(rif_config.ifindex))
                    {
                        sprintf(buff + strlen(buff),"TUNNEL %-8ld",tid);
                    }
                    else
#endif
                    sprintf(buff + strlen(buff),"VLAN %-10ld",(long)vid);

                    sprintf(buff + strlen(buff),
                    " "
                    "%-10s"
                    " "
                    "%-10s"
                    " "
                    "%s\r\n"
                    ,
                    str_vlan_status,/* Vlan status */
                    str_ipv6_status,/* IPv6 status */
                    "Unassigned"
                    );
#if 0
                    sprintf(buff + strlen(buff),
                    "VLAN %-10ld"
                    " "
                    "%-10s"
                    " "
                    "%-10s"
                    " "
                    "%s\r\n"
                    ,
                    vid,            /* Vlan */
                    str_vlan_status,/* Vlan status */
                    str_ipv6_status,/* IPv6 status */
                    "Unassigned"
                    );
#endif
                    PROCESS_MORE(buff);

	            } /* if */
	        } /* brief */
	    } /* while */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
        /*show craft interface */
        {
            NETCFG_TYPE_CraftInetAddress_T link_local, global;
            BOOL_T ipv6_enable = FALSE;
            BOOL_T has_ll_address = FALSE;
            BOOL_T has_global_address = FALSE;

            memset(&link_local, 0, sizeof(link_local));
            memset(&global, 0, sizeof(global));
            link_local.addr.type = L_INET_ADDR_TYPE_IPV6Z;
            if(NETCFG_TYPE_OK == NETCFG_POM_IP_GetCraftInterfaceInetAddress(&link_local))
                has_ll_address = TRUE;

            global.addr.type = L_INET_ADDR_TYPE_IPV6;
            if(NETCFG_TYPE_OK == NETCFG_POM_IP_GetCraftInterfaceInetAddress(&global))
                has_global_address = TRUE;

            NETCFG_POM_IP_GetIPv6EnableStatus_Craft(3, &ipv6_enable);

            /* IPv6 Up/Down, for brief mode */
            sprintf(str_ipv6_status, "%s", ipv6_enable? "Up":"Down");

            if(!brief)
            {
                if(has_ll_address || has_global_address)
                {
                    CLI_LIB_PrintStr("Craft interface is Administrative Up\r\n");

                    sprintf(buff + strlen(buff), "IPv6 is %s.\r\n", ipv6_enable? "enable":"stale");
                    PROCESS_MORE(buff);
                    sprintf(buff + strlen(buff),"Link-local address:\r\n");
                    PROCESS_MORE(buff);
                    if(has_ll_address)
                    {
                        L_INET_InaddrToString((L_INET_Addr_T *)&link_local.addr, addr_buf, sizeof(addr_buf));

                        sprintf(buff + strlen(buff),"  %s\r\n", addr_buf);
                    }
                    else
                    {
                        sprintf(buff + strlen(buff),"(None)\r\n");
                    }
                    PROCESS_MORE(buff);
                    /* Global unicast address(es) */
                    sprintf(buff + strlen(buff),"Global unicast address(es):\r\n");
                    PROCESS_MORE(buff);

                    if(has_global_address)
                    {
                        L_INET_InaddrToString((L_INET_Addr_T *)&global.addr, addr_buf, sizeof(addr_buf));

                        L_INET_AddrIp_T addr_tmp;

                        memcpy(&addr_tmp, &(global.addr), sizeof(addr_tmp));
                        /* ipv6 perfix address*/
                        if (TRUE == IP_LIB_GetPrefixAddr(global.addr.addr, global.addr.addrlen, global.addr.preflen, addr_tmp.addr))
                        {
                            L_INET_InaddrToString((L_INET_Addr_T *)&addr_tmp, str_ipv6_prefix, sizeof(str_ipv6_prefix));
                        }

                        sprintf(buff + strlen(buff),"  %s, subnet is %s", addr_buf, str_ipv6_prefix);
                        /* EUI-64 */
                        if(global.ipv6_addr_type == NETCFG_TYPE_IPV6_ADDRESS_TYPE_EUI64)
                            sprintf(str_addr_status, "%s", "EUI");

                        sprintf(buff + strlen(buff), "\r\n");
                        PROCESS_MORE(buff);

                    }
                    else
                    {
                        sprintf(buff + strlen(buff),"(None)\r\n");
                        PROCESS_MORE(buff);
                    }
                    /* Joined group address(es) */

                    sprintf(buff + strlen(buff), "Joined group address(es):\r\n");
                    PROCESS_MORE(buff);
                    memset(&maddr, 0, sizeof(maddr));
                    num_maddr = 0;
                    while(NETCFG_TYPE_OK == NETCFG_PMGR_IP_GetNextIfJoinIpv6McastAddr(SYS_ADPT_CRAFT_INTERFACE_IFINDEX, &maddr))
                    {
                        num_maddr++;
                        memset(addr_buf, 0, sizeof(addr_buf));
                        L_INET_InaddrToString((L_INET_Addr_T *)&maddr, addr_buf, sizeof(addr_buf));

                        sprintf(buff + strlen(buff),"%s\r\n",addr_buf);
                        PROCESS_MORE(buff);
                    }
                    if(!num_maddr)
                    {

                        sprintf(buff + strlen(buff), "(None)\r\n");
                        PROCESS_MORE(buff);
                    }
                    /* interface MTU */
                    if(NETCFG_PMGR_IP_GetIPv6InterfaceMTU(SYS_ADPT_CRAFT_INTERFACE_IFINDEX, &ipv6_mtu) == NETCFG_TYPE_OK)
                    {

                        sprintf(buff + strlen(buff), "IPv6 link MTU is %ld bytes\r\n", (long)ipv6_mtu);
                        PROCESS_MORE(buff);
                    }
                } // has_address
            }
            else
            {
                /* brief */
                if(has_ll_address)
                {
                    L_INET_InaddrToString((L_INET_Addr_T *)&link_local.addr, addr_buf, sizeof(addr_buf));
                    sprintf(buff,"%-15s", "Craft");
                    sprintf(buff + strlen(buff),
                    " "
                    "%-10s"
                    " "
                    "%-10s"
                    " "
                    "%s\r\n"
                    ,
                    "Up",           /* Interface status is always up*/
                    str_ipv6_status,/* IPv6 status */
                    addr_buf        /* IPv6 addresses */
                    );
                    PROCESS_MORE(buff);
                }
                if(has_global_address)
                {
                    L_INET_InaddrToString((L_INET_Addr_T *)&global.addr, addr_buf, sizeof(addr_buf));
                    sprintf(buff,"%-15s", "Craft");
                    sprintf(buff + strlen(buff),
                    " "
                    "%-10s"
                    " "
                    "%-10s"
                    " "
                    "%s\r\n"
                    ,
                    "Up",           /* Interface status is always up*/
                    str_ipv6_status,/* IPv6 status */
                    addr_buf        /* IPv6 addresses */
                    );
                    PROCESS_MORE(buff);
                }
                if(!has_ll_address && !has_global_address)
                {
                    sprintf(buff,"%-15s", "Craft");
                    sprintf(buff + strlen(buff),
                    " "
                    "%-10s"
                    " "
                    "%-10s"
                    " "
                    "%s\r\n"
                    ,
                    "Up",           /* Interface status is always up*/
                    str_ipv6_status,/* IPv6 status */
                    "Unassigned"
                    );
                    PROCESS_MORE(buff);
                }

            }
        }
#endif /* SYS_CPNT_CRAFT_PORT */
    }
    else /* with specified vlan ifindex(brief mode)*/
    {
        VLAN_VID_CONVERTTO_IFINDEX(vlan_id, l3_intf.ifindex);

        if(NETCFG_TYPE_OK == NETCFG_POM_IP_GetL3Interface(&l3_intf))
        {
            int num_ipv6_addr = 0;


            memset(str_vlan_status, 0, sizeof(str_vlan_status));
            memset(str_ipv6_status, 0, sizeof(str_ipv6_status));
#if 0       /*we can't let user display with specified tunnel id */
#if (SYS_CPNT_IP_TUNNEL == TRUE)
            if(IS_TUNNEL_IFINDEX(l3_intf.ifindex))
            {
                IP_LIB_ConvertTunnelIdFromIfindex(l3_intf.ifindex, &tid);
                op_index = 1;   /*  down    */
                INFOprintf("Get row_status= %d", l3_intf.u.tunnel_intf.row_status);
                if(l3_intf.u.tunnel_intf.row_status == VAL_netConfigStatus_2_active)
                {
                    op_index = 0;   /*  up  */
                }
            }
            else
#endif /*SYS_CPNT_IP_TUNNEL*/
#endif
            {

                memset(&vlan_entry, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
                /* Vlan up/down info. */
                if (VLAN_POM_GetDot1qVlanCurrentEntry(0,vlan_id, &vlan_entry)==FALSE)
                    op_index = 2;   /*  none    */
                else
                {
                    if (vlan_entry.if_entry.vlan_operation_status == VAL_ifOperStatus_up )
                        op_index = 0;   /*  up  */
                    else
                        op_index = 1;   /*  down    */
                }
            }

            /* Vlan Up/Down */
            switch (op_index)
            {
                case 0:
                    sprintf(str_vlan_status, "%s", "Up");
                    break;
                case 1:
                case 2:
                default:
                    sprintf(str_vlan_status, "%s", "Down");
                    break;
            }

            /* IPv6 Up/Down */
            sprintf(str_ipv6_status, "%s", (l3_intf.ipv6_enable)? "Up":"Down");

            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.addr.type = L_INET_ADDR_TYPE_IPV6;
            rif_config.ifindex = l3_intf.ifindex;
            while(NETCFG_POM_IP_GetNextInetRifOfInterface(&rif_config) == NETCFG_TYPE_OK)
            {
                char *char_p;

                if((rif_config.addr.type != L_INET_ADDR_TYPE_IPV6)
                    && (rif_config.addr.type != L_INET_ADDR_TYPE_IPV6Z))
                    break;

                num_ipv6_addr ++;

                /* IPv6 addresses */
                L_INET_InaddrToString((L_INET_Addr_T *)&rif_config.addr, addr_buf, sizeof(addr_buf));

                /* truncate zone-id(%N) for link-local address */
                char_p = strchr((char *)addr_buf, '%');
                if(char_p)
                {
                    /* link-local address */
                    *char_p = '\0';
                    sprintf((char *)addr_buf + strlen((char *)addr_buf), "/%d", rif_config.addr.preflen);
                }

                sprintf(buff + strlen(buff),
                        "VLAN %-10ld"
                        " "
                        "%-10s"
                        " "
                        "%-10s"
                        " "
                        "%s\r\n"
                        ,
                        (long)vlan_id,            /* Vlan */
                        str_vlan_status,/* Vlan status */
                        str_ipv6_status,/* IPv6 status */
                        addr_buf        /* IPv6 addresses */
                        );
                  PROCESS_MORE(buff);

            } /* while */

            if(!num_ipv6_addr)
            {

                sprintf(buff + strlen(buff),
                        "VLAN %-10ld"
                        " "
                        "%-10s"
                        " "
                        "%-10s"
                        " "
                        "%s\r\n"
                        ,
                        (long)vlan_id,            /* Vlan */
                        str_vlan_status,/* Vlan status */
                        str_ipv6_status,/* IPv6 status */
                        "Unassigned"
                        );
                PROCESS_MORE(buff);

            } /* if */
        }
    }
#endif /* SYS_CPNT_IPV6 */

    return (UI32_T)CLI_NO_ERROR;
}


#if 0 /* output example */
MTU     Since    Destination Address
1400    00:04:21 5000:1::3
1280    00:04:50 FE80::203:A0FF:FFD6:141D
#endif
UI32_T  CLI_API_Show_Ipv6_Mtu(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    NETCFG_TYPE_PMTU_Entry_T entry;
    char                   tmp_buf[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
    UI32_T  second, minute, hour,tmp;

    CLI_LIB_PrintStr(
    "MTU   "                       /* 6  */
    " "
    "Since     "                   /* 10 */
    " "
    "Destination Address\r\n"       /* 39 */
    );

    memset(&entry, 0, sizeof(entry));
    while(NETCFG_TYPE_OK == NETCFG_PMGR_IP_GetNextPMTUEntry(&entry))
    {

        second = entry.since_time % 60;
        tmp = entry.since_time / 60;         /* in minutes   */

        minute = tmp % 60;
        hour = tmp / 60;

        memset(tmp_buf, 0, sizeof(tmp_buf));
        L_INET_InaddrToString((L_INET_Addr_T *)&entry.destip, tmp_buf, sizeof(tmp_buf));

        #if 1
        CLI_LIB_PrintStr_4(
        "%-6ld"                   /* 6 */
        " "
        "%02lu:%02lu:%02lu  "     /* 10 */
        " ",
        (long)entry.pmtu,
        (unsigned long)hour, (unsigned long)minute, (unsigned long)second);
        #endif
        CLI_LIB_PrintStr_1("%s\r\n", tmp_buf);
    }
    return (UI32_T)CLI_NO_ERROR;
}

/* FUNTION NAME: CLI_API_Show_Ipv6_Traffic
 * PURPOSE:
 *      Display the ipv6 traffic counter.
 *      The API of command "show ipv6 traffic".
 *
 * INPUT:
 *      cmd_idx     -- command index
 *      arg         -- available addtional parameters array
 *      ctrl_P      -- CLI work space structure
 *
 * OUTPUT:
 *      None.
 * NOTES:
 *      None.
 */
#if 0 /* output example */
IPv6 Statistics:
IPv6 recived
                3886 rcvd total total received
                   0 header errors
IPv6 sent
                   0 forwarded datagrams
ICMPv6 Statistics:
ICMPv6 received
                3885 input
ICMPv6 sent
                  12 output
UDP Statistics:
                   1 input

#endif
UI32_T  CLI_API_Show_Ipv6_Traffic(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6 == TRUE)
    IPAL_Ipv6Statistics_T       ip6stat;
    IPAL_Icmpv6Statistics_T     icmp6stat;
    IPAL_Udpv6Statistics_T      udp6stat;

    memset(&ip6stat, 0, sizeof(ip6stat));
    memset(&icmp6stat, 0, sizeof(icmp6stat));
    memset(&udp6stat, 0, sizeof(udp6stat));

    /* get IPv6 statistics */
    if (NETCFG_TYPE_OK != NETCFG_PMGR_IP_GetIPv6Statistics(&ip6stat))
    {
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr("IPv6 Statistics:\n");

    /* IPv6 received statistic */
    CLI_LIB_PrintStr("IPv6 received\n");
    CLI_LIB_PrintStr_2("%20.0ld %s\r\n", (long)ip6stat.Ip6InReceives, "total received");
    CLI_LIB_PrintStr_2("%20.0ld %s\r\n", (long)ip6stat.Ip6InHdrErrors, "header errors");
    CLI_LIB_PrintStr_2("%20.0ld %s\r\n", (long)ip6stat.Ip6InTooBigErrors, "too big errors");
    CLI_LIB_PrintStr_2("%20.0ld %s\r\n", (long)ip6stat.Ip6InNoRoutes, "no routes");
    CLI_LIB_PrintStr_2("%20.0ld %s\r\n", (long)ip6stat.Ip6InAddrErrors, "address errors");
    CLI_LIB_PrintStr_2("%20.0ld %s\r\n", (long)ip6stat.Ip6InUnknownProtos, "unknown protocols");
    CLI_LIB_PrintStr_2("%20.0ld %s\r\n", (long)ip6stat.Ip6InTruncatedPkts, "truncated packets");
    CLI_LIB_PrintStr_2("%20.0ld %s\r\n", (long)ip6stat.Ip6InDiscards, "discards");
    CLI_LIB_PrintStr_2("%20.0ld %s\r\n", (long)ip6stat.Ip6InDelivers, "delivers");

    CLI_LIB_PrintStr_2("%20.0ld %s\r\n", (long)ip6stat.Ip6ReasmReqds, "reassembly request datagrams");
    CLI_LIB_PrintStr_2("%20.0ld %s\r\n", (long)ip6stat.Ip6ReasmOKs, "reassembly succeeded");
    CLI_LIB_PrintStr_2("%20.0ld %s\r\n", (long)ip6stat.Ip6ReasmFails, "reassembly failed");

    /* IPv6 sent statistic */

    CLI_LIB_PrintStr("IPv6 sent\n");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ip6stat.Ip6OutForwDatagrams, "forwards datagrams");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ip6stat.Ip6OutRequests, "requests");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ip6stat.Ip6OutDiscards, "discards");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ip6stat.Ip6OutNoRoutes, "no routes");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ip6stat.Ip6FragCreates, "generated fragments");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ip6stat.Ip6FragOKs, "fragment succeeded");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ip6stat.Ip6FragFails, "fragment failed");

    /* get ICMPv6 statistics */
    if (NETCFG_TYPE_OK != NETCFG_PMGR_IP_GetICMPv6Statistics(&icmp6stat))
    {
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr("ICMPv6 Statistics:\n");

    /* ICMPv6 received statistics */
    CLI_LIB_PrintStr("ICMPv6 received\n");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InMsgs, "input");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InErrors, "errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InDestUnreachs, "destination unreachable messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InPktTooBigs, "packet too big messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InTimeExcds, "time exceeded messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InParmProblems, "parameter problem message");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InEchos, "echo request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InEchoReplies, "echo reply messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InRouterSolicits, "router solicit messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InRouterAdvertisements, "router advertisement messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InNeighborSolicits, "neighbor solicit messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InNeighborAdvertisements, "neighbor advertisement messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InRedirects, "redirect messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InGroupMembQueries, "group membership query messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InGroupMembResponses, "group membership response messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6InGroupMembReductions, "group membership reduction messages");

    /* ICMPv6 sent statistics */
    CLI_LIB_PrintStr("ICMPv6 sent\n");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutMsgs, "output");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutDestUnreachs, "destination unreachable messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutPktTooBigs, "packet too big messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutTimeExcds, "time exceeded messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutParmProblems, "parameter problem message");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutEchos, "echo request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutEchoReplies, "echo reply messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutRouterSolicits, "router solicit messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutRouterAdvertisements, "router advertisement messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutNeighborSolicits, "neighbor solicit messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutNeighborAdvertisements, "neighbor advertisement messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutRedirects, "redirect messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutGroupMembQueries, "group membership query messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutGroupMembResponses, "group membership response messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmp6stat.Icmp6OutGroupMembReductions, "group membership reduction messages");

    /* get UDP statistics */
    if (NETCFG_TYPE_OK != NETCFG_PMGR_IP_GetUDPv6Statistics(&udp6stat))
    {
        return CLI_NO_ERROR;
    }

    /* UDP statistics */
    CLI_LIB_PrintStr("UDP Statistics:\n");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)udp6stat.Udp6InDatagrams, "input");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)udp6stat.Udp6NoPorts, "no port errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)udp6stat.Udp6InErrors, "other errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)udp6stat.Udp6OutDatagrams, "output");

    /* TCP statistic is missing in linux */

#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

    return (UI32_T)CLI_NO_ERROR;
}

/* FUNTION NAME: CLI_API_Clear_Ipv6_Traffic
 * PURPOSE:
 *      Reset the ipv6 traffic counters
 *
 * INPUT:
 *      cmd_idx     -- command index
 *      arg         -- available addtional parameters array
 *      ctrl_P      -- CLI work space structure
 *
 * OUTPUT:
 *      None.
 * NOTES:
 *      None.
 */
UI32_T  CLI_API_Clear_Ipv6_Traffic(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6 == TRUE)
    if (NETCFG_TYPE_OK != NETCFG_PMGR_IP_ClearIPv6StatisticsByType(
                NETCFG_TYPE_IPV6_STAT_TYPE_ALL))
    {
        CLI_LIB_PrintStr("\r\nFailed to clear IPv6 traffic statistics.\r\n");
    }
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

    return (UI32_T)CLI_NO_ERROR;
}

/* cmd: ping6 */
UI32_T  CLI_API_Ping_Ipv6(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#define INIT_MAX_DELTA_TIME 0
#define INIT_MIN_DELTA_TIME 0xffffffff
//#define MAX_BUF_LEN         45

    UI32_T count, ret;
    UI32_T size, i;
    UI32_T timeout = SYS_DFLT_PING_CTL_TIME_OUT; /* in system tick unit */
    UI32_T success_num = 0;
    UI32_T send_num = 0;
    UI32_T max_delta_time = INIT_MAX_DELTA_TIME;
    UI32_T min_delta_time = INIT_MIN_DELTA_TIME;
    UI32_T total_success_delta_time = 0;
    PING_TYPE_PingCtlEntry_T    ctl_entry;
    PING_TYPE_PingResultsEntry_T result_entry;
    PING_TYPE_PingProbeHistoryEntry_T history_entry, printed_history_entry;
    UI32_T pom_rc;
    UI8_T session_id;

    L_INET_AddrIp_T inet_addr;
    char tmp_buf[L_INET_MAX_IP6ADDR_STR_LEN+1] = {0};
    UI32_T start_time, previous_history_time;
    BOOL_T stop = FALSE;

    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;

    /* to check the input is IPv6 format
     */
    if (L_INET_RETURN_SUCCESS == L_INET_StringToInaddr(L_INET_FORMAT_IPV4_UNSPEC,
                                                       arg[0],
                                                       (L_INET_Addr_T *)&inet_addr,
                                                       sizeof(inet_addr))
        ||
        L_INET_RETURN_SUCCESS == L_INET_StringToInaddr(L_INET_FORMAT_IPV6_PREFIX,
                                                       arg[0],
                                                       (L_INET_Addr_T *)&inet_addr,
                                                       sizeof(inet_addr))
       )
    {
        CLI_LIB_PrintStr_1("Invalid IPv6 address - %s\r\n", arg[0]);
        return CLI_NO_ERROR;
    }

    size  = SYS_ADPT_MIN_PING6_SIZE; /* 32 */
    count = 5;

    for(i=1; arg[i]; )
    {
        if (arg[i][0] == 'c') /* count */
        {
            count = atoi(arg[i+1]);
            i+=2;
        }
        else if (arg[i][0] == 's') /* size */
        {
            size = atoi(arg[i+1]);
            i+=2;
        }
        else /* this should not happen */
        {
            CLI_LIB_PrintStr_1("Unknown argument %s.\r\n", arg[i]);
            return CLI_NO_ERROR;
        }
    } /* for */

    /* for multiple telnet console, they should use different owner_index and test_name from taskid. */
    session_id = CLI_TASK_GetMySessId();

    /* DEBUG */
    //printf("%s, %d, default_string: %s\n", __FUNCTION__, __LINE__, default_string);

    /* initialize entry */
    memset(&ctl_entry, 0 , sizeof(PING_TYPE_PingCtlEntry_T));
    memset(&result_entry, 0 , sizeof(PING_TYPE_PingResultsEntry_T));
    memset(&history_entry, 0 , sizeof(PING_TYPE_PingProbeHistoryEntry_T));

    strncpy(ctl_entry.ping_ctl_owner_index,
        PING_TYPE_CTL_OWNER_INDEX_SYSTEM_CLI,
        sizeof(ctl_entry.ping_ctl_owner_index) - 1);
    ctl_entry.ping_ctl_owner_index[sizeof(ctl_entry.ping_ctl_owner_index) - 1] = '\0';
    ctl_entry.ping_ctl_owner_index_len = strlen((char *)ctl_entry.ping_ctl_owner_index);

    snprintf((char *)ctl_entry.ping_ctl_test_name, sizeof(ctl_entry.ping_ctl_test_name),
        "%d", session_id);
    ctl_entry.ping_ctl_test_name[sizeof(ctl_entry.ping_ctl_test_name) - 1] = '\0';
    ctl_entry.ping_ctl_test_name_len = strlen((char *)ctl_entry.ping_ctl_test_name);

    strncpy(result_entry.ping_ctl_owner_index,
        PING_TYPE_CTL_OWNER_INDEX_SYSTEM_CLI,
        sizeof(result_entry.ping_ctl_owner_index) - 1);
    result_entry.ping_ctl_owner_index[sizeof(result_entry.ping_ctl_owner_index) - 1] = '\0';
    result_entry.ping_ctl_owner_index_len = strlen((char *)result_entry.ping_ctl_owner_index);

    snprintf((char *)result_entry.ping_ctl_test_name, sizeof(result_entry.ping_ctl_test_name),
        "%d", session_id);
    result_entry.ping_ctl_test_name[sizeof(result_entry.ping_ctl_test_name) - 1] = '\0';
    result_entry.ping_ctl_test_name_len = strlen((char *)result_entry.ping_ctl_test_name);

    /* admin status is disabled firstly. */
    ctl_entry.ping_ctl_admin_status = VAL_pingCtlAdminStatus_disabled;
    /* size */
    ctl_entry.ping_ctl_data_size = size;
    /* count */
    ctl_entry.ping_ctl_probe_count = count;

    /* default time-out 5 sec.*/
    ctl_entry.ping_ctl_timeout = (SYS_DFLT_PING_CTL_TIME_OUT / SYS_BLD_TICKS_PER_SECOND);

    /* create entry */
    ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_createAndGo;

    if(PING_TYPE_OK != PING_PMGR_SetCtlEntry(&ctl_entry))
    {
        CLI_LIB_PrintStr((char *)"Not enough resource; Please try later\r\n");

        /* silently destroy ctl entry */
        ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
        PING_PMGR_SetCtlEntry(&ctl_entry);
        return CLI_NO_ERROR;
    }
    /* display message */
    CLI_LIB_PrintStr((char *)"Press \"ESC\" to abort.\r\n");

    CLI_LIB_PrintStr_1("Ping to %s", arg[0]);

    memset(&inet_addr, 0, sizeof(inet_addr));

    start_time = SYSFUN_GetSysTick();

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC,
                                                       arg[0],
                                                       (L_INET_Addr_T *) &inet_addr,
                                                       sizeof(inet_addr)))
    {
    #if (SYS_CPNT_DNS == TRUE)
        DNS_Nslookup_CTRL_T nslookup_ctl_entry;
        DNS_Nslookup_Result_T nslookup_result_entry;
        UI32_T nslookup_ctl_index;
        UI32_T nslookup_result_index;

        memset(&nslookup_ctl_entry, 0, sizeof(nslookup_ctl_entry));

        strncpy((char *)nslookup_ctl_entry.CtlOwnerIndex,
            DNS_TYPE_NSLOOKUP_CTL_OWNER_INDEX_SYSTEM_CLI,
            sizeof(nslookup_ctl_entry.CtlOwnerIndex) - 1);
        nslookup_ctl_entry.CtlOwnerIndex[sizeof(nslookup_ctl_entry.CtlOwnerIndex) - 1] = '\0';
        nslookup_ctl_entry.CtlOwnerIndexLen = strlen((char *)nslookup_ctl_entry.CtlOwnerIndex);

        snprintf((char *)nslookup_ctl_entry.OperationName, sizeof(nslookup_ctl_entry.OperationName),
            "%d", CLI_TASK_GetMySessId());
        nslookup_ctl_entry.OperationName[sizeof(nslookup_ctl_entry.OperationName) - 1] = '\0';
        nslookup_ctl_entry.OperationNameLen = strlen((char *)nslookup_ctl_entry.OperationName);

        strncpy((char *)nslookup_ctl_entry.TargetAddress,
            arg[0],
            sizeof(nslookup_ctl_entry.TargetAddress)-1);
        nslookup_ctl_entry.TargetAddress[sizeof(nslookup_ctl_entry.TargetAddress) - 1] = '\0';
        nslookup_ctl_entry.TargetAddressLen = strlen((char *)nslookup_ctl_entry.TargetAddress);

        nslookup_ctl_entry.af_family = AF_INET6;

        if (DNS_OK != DNS_PMGR_CreateSystemNslookupCtlEntry(&nslookup_ctl_entry, &nslookup_ctl_index))
        {
            CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
            ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
            PING_PMGR_SetCtlEntry(&ctl_entry);
            return CLI_NO_ERROR;
        }

        while (1)
        {
            if (CLI_IO_ReadACharNoWait() == 0x1B) /*ESC*/
            {
                UI8_T tmp_char;
                /* EPR ID:ES4626F-SW-FLF-38-00595
                 * Arrow keys are consisted by 3 characters
                 * and the 1st character is 0x1B
                 * Users are not happy to see the latter 2 characters being
                 * printed. So do the workaround here to remove these 2 characters
                 * from Telnet rx buffer to avoid being printed by CLI latter
                 * Cross reference to CLI_IO_GetKeyByVtTerminal() in cli_io.c
                 */

                tmp_char = CLI_IO_ReadACharNoWait(ctrl_P);
                if (tmp_char!=0)
                {
                    if (tmp_char==0x5B || tmp_char==0x4F) /* 2nd char of arrow key */
                    {
                        /* read the last char to clear arrow key in buffer
                         */
                        CLI_IO_ReadACharNoWait(ctrl_P);
                    }
                }

                /* silently destroy ctl entry */
                CLI_LIB_Printf("\r\n");
                DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
                PING_PMGR_SetCtlEntry(&ctl_entry);
                return CLI_NO_ERROR;
            }

            memset(&nslookup_ctl_entry, 0, sizeof(nslookup_ctl_entry));

            if (DNS_OK == DNS_PMGR_GetNslookupCtlEntryByIndex(nslookup_ctl_index, &nslookup_ctl_entry))
            {
                if (NSLOOKUP_OPSTATUS_COMPLETED == nslookup_ctl_entry.OperStatus)
                {
                    if (DNS_OK == nslookup_ctl_entry.Rc)
                    {
                        nslookup_result_index = 0;
                        memset(&nslookup_result_entry, 0, sizeof(nslookup_result_entry));

                        if (DNS_OK == DNS_PMGR_GetNslookupResultEntryByIndex(
                            nslookup_ctl_index, nslookup_result_index, &nslookup_result_entry))
                        {
                            memcpy(&inet_addr, &nslookup_result_entry.ResultsAddress_str, sizeof(inet_addr));
                            DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                            break;
                        }
                    }

                    CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
                    DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                    ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
                    PING_PMGR_SetCtlEntry(&ctl_entry);
                    return CLI_NO_ERROR;
                }
            }
            else
            {
                CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
                ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
                PING_PMGR_SetCtlEntry(&ctl_entry);
                return CLI_NO_ERROR;
            }

            /* To avoid somthing wrong, CLI maybe halt, we wait a bit longer than total timeout, then break */
            if((SYSFUN_GetSysTick() - start_time) > timeout * (count+1))
            {
                CLI_LIB_PrintStr_1("\r\nTimeout for lookup host name %s.\r\n", arg[0]);
                DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
                PING_PMGR_SetCtlEntry(&ctl_entry);
                return CLI_NO_ERROR;
            }

            SYSFUN_Sleep(50);
        }

        /* display ipv6 address got from DNS */
        L_INET_InaddrToString((L_INET_Addr_T *) &inet_addr, tmp_buf, sizeof(tmp_buf));
        CLI_LIB_PrintStr_1(" [%s]", tmp_buf);
    #else
        /* No DNS
         */
        CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
        ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
        PING_PMGR_SetCtlEntry(&ctl_entry);
        return CLI_NO_ERROR;
    #endif

    }

    /* address type */
    ctl_entry.ping_ctl_target_address_type = inet_addr.type;
    ret = PING_PMGR_SetCtlEntryByField(&ctl_entry, PING_TYPE_CTLENTRYFIELD_TARGET_ADDRESS_TYPE);

    if(PING_TYPE_OK != ret)
    {
        CLI_LIB_PrintStr("\r\nInvalid address type.\r\n");
        /* silently destroy ctl entry */
        ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
        PING_PMGR_SetCtlEntry(&ctl_entry);
        return CLI_NO_ERROR;
    }

    /* link-local address must has zone-id (%N) */
    if(L_INET_ADDR_IS_IPV6_LINK_LOCAL(inet_addr.addr))
    {
        if(strchr((char*)arg[0], '%') == NULL)
        {
            CLI_LIB_Printf("\r\nInvalid address format.\r\n");
            CLI_LIB_Printf("Please specify a ZoneID(%%N) for link-local or multicast address.\r\n");
            /* silently destroy ctl entry */
            ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
            PING_PMGR_SetCtlEntry(&ctl_entry);
            return CLI_NO_ERROR;
        }
    }

    CLI_LIB_PrintStr_3(", by %lu %lu-byte payload ICMP packets, timeout is %lu seconds\r\n", (unsigned long)count, (unsigned long)size, (unsigned long)(timeout / SYS_BLD_TICKS_PER_SECOND));

    /* DEBUG */
    /* ret = L_INET_InaddrToString((L_INET_Addr_T *) &inet_addr, tmp_buf, sizeof(tmp_buf));
    printf("ret: %ld\n", ret);
    printf("inet_addr %s\n", tmp_buf);
    */

    ret = PING_PMGR_SetCtlTargetAddress(&ctl_entry, &inet_addr);
    if(PING_TYPE_OK != ret)
    {
        if (PING_TYPE_IPV6_LINK_LOCAL_ADDR_INVALID_ZONE_ID == ret)
        {
#if (SYS_CPNT_CRAFT_PORT == TRUE)
            UI32_T craft_id = (SYS_ADPT_CRAFT_INTERFACE_IFINDEX - SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1);
            CLI_LIB_PrintStr_1("Invalid zone ID for IPv6 link-local address. Zone ID should be a valid VLAN ID or Craft interface ID(%lu).\r\n", (unsigned long)craft_id);
#else
            CLI_LIB_PrintStr("Invalid zone ID for IPv6 link-local address. Zone ID should be a valid VLAN ID.\r\n");
#endif
        }
        else 
            CLI_LIB_PrintStr("\r\nInvalid address.\r\n");
        /* silently destroy ctl entry */
        ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
        PING_PMGR_SetCtlEntry(&ctl_entry);
        return CLI_NO_ERROR;
    }

    /* DEBUG */
    //printf("%s, %d, PING_PMGR_SetCtlTargetAddress ret : 0x%lx\n", __FUNCTION__, __LINE__, ret);

    /* enable admin status to start to ping.*/
    ret = PING_PMGR_SetCtlAdminStatus(&ctl_entry, VAL_pingCtlAdminStatus_enabled);

    /* DEBUG */
    //printf("%s, %d, PING_PMGR_SetCtlAdminStatus ret : 0x%lx\n", __FUNCTION__, __LINE__, ret);

    if(PING_TYPE_OK != ret)
    {
        /* silently destroy ctl entry */
        ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
        PING_PMGR_SetCtlEntry(&ctl_entry);
        return CLI_NO_ERROR;
    }

    /* initialize entry */
    strncpy(history_entry.ping_ctl_owner_index,
        PING_TYPE_CTL_OWNER_INDEX_SYSTEM_CLI,
        sizeof(history_entry.ping_ctl_owner_index) - 1);
    history_entry.ping_ctl_owner_index[sizeof(history_entry.ping_ctl_owner_index) - 1] = '\0';
    history_entry.ping_ctl_owner_index_len = strlen((char *)history_entry.ping_ctl_owner_index);

    snprintf((char *)history_entry.ping_ctl_test_name, sizeof(history_entry.ping_ctl_test_name),
        "%d", session_id);
    history_entry.ping_ctl_test_name[sizeof(history_entry.ping_ctl_test_name) - 1] = '\0';
    history_entry.ping_ctl_test_name_len = strlen((char *)history_entry.ping_ctl_test_name);

    history_entry.ping_probe_history_index = 0;
    memcpy(&printed_history_entry, &history_entry, sizeof(history_entry));

    //count_printed = 0;
    previous_history_time = SYSFUN_GetSysTick();
    while(1)
    {
        /* break when ESC is pressed */
        if(ctrl_P->sess_type == CLI_TYPE_TELNET)
        {
            if( CLI_IO_ReadACharFromTelnet(ctrl_P) == 0x1B) /*ESC*/
            {
                PING_PMGR_SetCtlAdminStatus(&ctl_entry, VAL_pingCtlAdminStatus_disabled);
                break;
            }
        }
        else
        {
            if( CLI_IO_ReadACharFromConsole(ctrl_P) == 0x1B) /*ESC*/
            {
                PING_PMGR_SetCtlAdminStatus(&ctl_entry, VAL_pingCtlAdminStatus_disabled);
                break;
            }
        }
        pom_rc = PING_POM_GetCtlEntry(&ctl_entry);

        if(PING_TYPE_OK == pom_rc)
        {
            while (PING_POM_GetNextProbeHistoryEntryForCli(&history_entry) == PING_TYPE_OK)
            {
                previous_history_time = history_entry.system_tick;

                if ((0 == strcmp(history_entry.ping_ctl_owner_index, ctl_entry.ping_ctl_owner_index)) &&
                    (0 == strcmp(history_entry.ping_ctl_test_name, ctl_entry.ping_ctl_test_name)))
                {

                    if(VAL_pingProbeHistoryStatus_responseReceived == history_entry.ping_probe_history_status)
                    {
                        /* accumulate total_success_delta_time */
                        total_success_delta_time += history_entry.ping_probe_history_response;
                        CLI_LIB_PrintStr_1("response time: %lu ms", (unsigned long)history_entry.ping_probe_history_response);

                        L_INET_InaddrToString((L_INET_Addr_T *) &history_entry.src_addr, tmp_buf, sizeof(tmp_buf));
                        /* 54 is L_INET_MAX_IP6ADDR_STR_LEN */
                        CLI_LIB_PrintStr_2("\t[%s]\tseq_no: %ld\r\n", tmp_buf, (long)history_entry.icmp_sequence);
                    }
                    else
                    {
                        /* time out */
                        CLI_LIB_PrintStr((char *)"timeout.\r\n");
                    }

                    //count_printed++;
                    memcpy(&printed_history_entry, &history_entry, sizeof(history_entry));
                }
                else
                {
                    memcpy(&history_entry, &printed_history_entry, sizeof(history_entry));
                }

            } /* while */

            if ((SYSFUN_GetSysTick() - previous_history_time > SYS_DFLT_PING_CTL_TIME_OUT)
                && (history_entry.ping_probe_history_index >= ctl_entry.ping_ctl_probe_count)) //&& (count_printed >= ctl_entry.ping_ctl_probe_count))
            {
                    /* DEBUG */
                //printf("%s, %d, %s\n", __FUNCTION__, __LINE__, "time-out, stoppppppp.");
                stop = TRUE;
                break;
            }
            /* To avoid somthing wrong, CLI maybe halt, we wait a bit longer than total timeout, then break */
            if((SYSFUN_GetSysTick() - start_time) > timeout * (count+1))
            {
                /* DEBUG */
                //printf("%s, %d, %s\n", __FUNCTION__, __LINE__, "stop due to total timeout");
                stop = TRUE;
                break;
            }

        } /* if PING_MGR_GetCtlEntry */
        if(stop)
            break;
        /* sleep for a while.  Because it is not necessary to query too frequently. */
        SYSFUN_Sleep(30);

    } /* while */

    /* display result and statistic */

    if(PING_TYPE_OK == PING_POM_GetResultsEntry(&result_entry))
    {

        send_num = result_entry.ping_results_sent_probes;
        success_num = result_entry.ping_results_probe_responses;
        min_delta_time = result_entry.ping_results_min_rtt;
        max_delta_time = result_entry.ping_results_max_rtt;

        CLI_LIB_PrintStr_1("Ping statistics for %s: \r\n", arg[0]);
        CLI_LIB_PrintStr_1(" %lu packets transmitted,", (unsigned long)send_num);
        CLI_LIB_PrintStr_4(" %lu packets received (%lu%%), %lu packets lost (%lu%%)\r\n",
            (unsigned long)success_num,
            (unsigned long)(send_num?success_num*100/send_num:0),
            (unsigned long)((send_num > success_num)? (send_num - success_num): 0),
            (unsigned long)((send_num > success_num)?(send_num - success_num)*100/send_num:0));
        CLI_LIB_PrintStr((char *)"Approximate round trip times: \r\n");
        CLI_LIB_PrintStr_3(" Minimum = %lu ms, Maximum = %lu ms, Average = %lu ms\r\n", (unsigned long)(min_delta_time == INIT_MIN_DELTA_TIME ? 0 : min_delta_time),
                           (unsigned long)(max_delta_time == INIT_MAX_DELTA_TIME ? 0 : max_delta_time),
                           (unsigned long)(success_num == 0 ? 0: (total_success_delta_time/success_num)));


    }
    else
    {
         CLI_LIB_PrintStr((char *)"Unknown error, get result entry error.\r\n");
    }

    /* destroy ctl entry */
    ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
    if(PING_TYPE_OK != PING_PMGR_SetCtlEntry(&ctl_entry))
    {
        CLI_LIB_PrintStr((char *)"Delete Ping Entry failed.\r\n");
        return CLI_NO_ERROR;
    }

    return CLI_NO_ERROR;
}
/* cmd: ipv6 default-gateway */
UI32_T  CLI_API_Ipv6_Default_Gateway(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   /*UI32_T ui32_ip;
   UI8_T byte_ip[SYS_ADPT_IPV6_ADDR_LEN];*/
   L_INET_AddrIp_T default_gateway;
   UI32_T rc;

   memset(&default_gateway, 0, sizeof(L_INET_AddrIp_T));
   default_gateway.type = L_INET_ADDR_TYPE_IPV6;
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
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_IPV6_DEFAULTGATEWAY:
            if(arg[0] == NULL)
            {
                CLI_LIB_Printf("Unknown error!\r\n");
                break;
            }

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC,
                                                               arg[0],
                                                               (L_INET_Addr_T *) &default_gateway,
                                                               sizeof(default_gateway)))
            {
                CLI_LIB_Printf("Invalid IPV6 address!\r\n");
                break;
            }

            /* link-local address must has zone-id (%N), which maybe 0, thus the string is checked. */
            /* check FE80::/64 by low-level function */
            if(L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL(default_gateway.addr))
            {
                if(strchr((char*)arg[0], '%') == NULL)
                {
                    CLI_LIB_Printf("Error. Link-local address must have ZoneID.\r\n");
                    return CLI_NO_ERROR;
                }
                /* else, keep going */
            }

            if(NETCFG_TYPE_OK != (rc = NETCFG_PMGR_ROUTE_AddDefaultGateway(&default_gateway, SYS_DFLT_DEFAULT_GATEWAY_METRIC)))
            {
                if(NETCFG_TYPE_IPV6_LINK_LOCAL_ADDR_INVALID_ZONE_ID == rc)
                {
#if (SYS_CPNT_CRAFT_PORT == TRUE)
                    UI32_T craft_id = (SYS_ADPT_CRAFT_INTERFACE_IFINDEX - SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1);
                    CLI_LIB_PrintStr_1("Invalid zone ID for IPv6 link-local address. Zone ID should be a valid VLAN ID or Craft interface ID(%lu).\r\n", (unsigned long)craft_id);
#else
                    CLI_LIB_PrintStr("Invalid zone ID for IPv6 link-local address. Zone ID should be a valid VLAN ID.\r\n");
#endif
                }
                else
                {
                    CLI_LIB_PrintStr("Failed to set default gateway.\r\n");
                }
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IPV6_DEFAULTGATEWAY:
            default_gateway.type = L_INET_ADDR_TYPE_IPV6;
            default_gateway.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
            NETCFG_PMGR_ROUTE_GetDefaultGateway(&default_gateway);
            if(NETCFG_PMGR_ROUTE_DeleteDefaultGateway(&default_gateway)!=NETCFG_TYPE_OK)
            {
                CLI_LIB_PrintStr("Failed to disable the default gateway.\r\n");
            }
            break;

        default:
            return CLI_NO_ERROR;
    }

    return (UI32_T)CLI_NO_ERROR;
}


UI32_T  CLI_API_L3_IPv6_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_ROUTING == TRUE)

//    UI8_T dst_netAddress[SYS_ADPT_IPV6_ADDR_LEN+3] = {0};
//    UI8_T netmask_Address[SYS_ADPT_IPV6_ADDR_LEN] = {0};
//    UI8_T gateway_Address[SYS_ADPT_IPV6_ADDR_LEN] = {0};
    UI32_T   distance        = SYS_ADPT_MIN_ROUTE_DISTANCE;
    UI8_T    choice[2] = {0};
    UI32_T   return_type = 0, action_flags;
    BOOL_T all_network = TRUE;
   //L_PREFIX_IPv6_T net_addr, gate_addr;
    L_INET_AddrIp_T dest, next_hop;
    UI32_T rc;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_IPV6_ROUTE:
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        if(arg[1][0]=='t' || arg[1][0]=='T')
        {
            UI32_T tidifindex;
            UI32_T vid=CLI_LIB_AtoUl(arg[2], 10);
            L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC, arg[0], (L_INET_Addr_T *)&dest, sizeof(dest));
            IP_LIB_ConvertTunnelIdToIfindex(vid, &tidifindex);
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ROUTE_AddStaticRouteToTunnel(&dest, tidifindex))
                CLI_LIB_PrintStr("Failed to configure the net-routing operation.\r\n");
            break;
        }
#endif/*SYS_CPNT_IP_TUNNEL*/
        L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC, arg[0], (L_INET_Addr_T *)&dest, sizeof(dest));
        L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC, arg[1], (L_INET_Addr_T *)&next_hop, sizeof(next_hop));

        /* link-local address must has zone-id (%N), which maybe 0, thus the string is checked. */
        /* check FE80::/64by low-level function */
        if(L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL(next_hop.addr))
        {
            if(strchr((char*)arg[1], '%') == NULL)
            {
                CLI_LIB_Printf("Error. Link-local address must have ZoneID.\r\n");
                return CLI_NO_ERROR;
            }
            /* else, keep going */
        }

        if (arg[3] != NULL)
        {
            distance = CLI_LIB_AtoUl(arg[2], 10);
        }

        if (NETCFG_TYPE_OK != (rc = NETCFG_PMGR_ROUTE_AddStaticRoute(&dest, &next_hop, distance)))
        {
            switch (rc)
            {
                case NETCFG_TYPE_ROUTE_PREFIX_LEN_MORE_THAN_64 : /* raised from NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry */
                    CLI_LIB_PrintStr("Failed. The route with prefix-length more than 64 is not supported.\r\n");
                    break;
                case NETCFG_TYPE_INVALID_NEXTHOP_LOCAL_IP:
                    CLI_LIB_PrintStr("Invalid nexthop. Local IP is not allowed.\r\n");
                    break;
                case NETCFG_TYPE_INVALID_NEXTHOP_IPV6_UNSPECIFIED:
                    CLI_LIB_PrintStr("Invalid nexthop. Unspecified address is not allowed.\r\n");
                    break;
                case NETCFG_TYPE_INVALID_NEXTHOP_IPV6_LOOPBACK:
                    CLI_LIB_PrintStr("Invalid nexthop. Loopback address is not allowed.\r\n");
                    break;
                case NETCFG_TYPE_INVALID_NEXTHOP_IPV6_MULTICAST:
                    CLI_LIB_PrintStr("Invalid nexthop. IPv6 multicast address is not allowed.\r\n");
                    break;
                case NETCFG_TYPE_INVALID_ROUTE_IPV6_UNSPECIFIED:
                    CLI_LIB_PrintStr("Invalid route. Unspecified address is not allowed.\r\n");
                    break;
                case NETCFG_TYPE_INVALID_ROUTE_IPV6_LOOPBACK:
                    CLI_LIB_PrintStr("Invalid route. Loopback address is not allowed.\r\n");
                    break;
                case NETCFG_TYPE_INVALID_ROUTE_IPV6_MULTICAST:
                    CLI_LIB_PrintStr("Invalid route. IPv6 multicast address is not allowed.\r\n");
                    break;
                case NETCFG_TYPE_INVALID_ROUTE_IPV6_LINKLOCAL:
                    CLI_LIB_PrintStr("Invalid route. IPv6 link-local address is not allowed.\r\n");
                    break;
                default: /* other errors */
                    CLI_LIB_PrintStr("Failed to configure the net-routing operation.\r\n");
                    break;
            }
        }
        break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IPV6_ROUTE:
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        if(arg[1][0]=='t' || arg[1][0]=='T')
        {
            UI32_T tidifindex;
            UI32_T vid=CLI_LIB_AtoUl(arg[2], 10);
            L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC, arg[0], (L_INET_Addr_T *)&dest, sizeof(dest));
            IP_LIB_ConvertTunnelIdToIfindex(vid, &tidifindex);
            if(NETCFG_TYPE_OK!=NETCFG_PMGR_ROUTE_DeleteStaticRouteToTunnel(&dest, tidifindex))
                CLI_LIB_PrintStr("Failed to configure the net-routing operation.\r\n");
            break;
        }
#endif/*SYS_CPNT_IP_TUNNEL*/
        if(arg[0][0] == '*')
        {
            CLI_LIB_PrintStr("This operation will delete all static entries in routing table.\r\n");
            CLI_LIB_PrintStr("Are you sure to continue this operation (y/n)?");
            CLI_PARS_ReadLine((char *) choice, sizeof(choice), TRUE, FALSE);
            CLI_LIB_PrintNullStr(1);

            if( choice[0] == 0 || (choice[0] != 'y' && choice[0] != 'Y') )
                return CLI_NO_ERROR;

            action_flags = L_INET_ADDR_TYPE_IPV6;
            return_type = NETCFG_PMGR_ROUTE_DeleteAllStaticIpCidrRoutes(action_flags);

            if ( return_type == NETCFG_TYPE_OK )
            {
                CLI_LIB_PrintStr( "The all static entries have been cleared.\r\n");
            }
            else
            {
                CLI_LIB_PrintStr( "Failed to clear all static entries.\r\n");
            }
        }
        else
        {
            L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC, arg[0], (L_INET_Addr_T *)&dest, sizeof(dest));

            if (arg[1] != NULL)
            {
                all_network = FALSE;
                L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC, arg[1], (L_INET_Addr_T *)&next_hop, sizeof(next_hop));

                /* link-local address must has zone-id (%N), which maybe 0, thus the string is checked. */
                /* check FE80::/64 by low-level function */
                if(L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL(next_hop.addr))
                {
                    if(strchr((char*)arg[1], '%') == NULL)
                    {
                        CLI_LIB_Printf("Error. Link-local address must have ZoneID.\r\n");
                        return CLI_NO_ERROR;
                    }
                    /* else, keep going */
                }
            }

            if (all_network == FALSE)
            {
                if (NETCFG_PMGR_ROUTE_DeleteStaticRoute(&dest, &next_hop)!=NETCFG_TYPE_OK)
                {
                    CLI_LIB_PrintStr("Failed to clear the entry.\r\n");
                }
            }
//             else if(NETCFG_PMGR_ROUTE_DeleteStaticRouteByNetwork(dst_ipAddress,netmask_Address)!=NETCFG_TYPE_OK)
//             {
//                CLI_LIB_PrintStr((UI8_T *) "Failed to clear the entries.\r\n");
//                return CLI_NO_ERROR;
//             }

        }
        break;
        default:
            return CLI_NO_ERROR;
    }
#endif /* SYS_CPNT_IPV6_ROUTING */

    return (UI32_T)CLI_NO_ERROR;
}

UI32_T  CLI_API_Show_Ipv6_Default_Gateway(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
// peter_yu, #if (SYS_CPNT_ROUTING == TRUE)
   L_INET_AddrIp_T default_gateway;
   default_gateway.type = L_INET_ADDR_TYPE_IPV6;
   char tmp_buf[L_INET_MAX_IPADDR_STR_LEN+1] = {0};


   if (NETCFG_PMGR_ROUTE_GetDefaultGateway(&default_gateway) == NETCFG_TYPE_OK)
   {
        L_INET_InaddrToString((L_INET_Addr_T *) &default_gateway, tmp_buf, sizeof(tmp_buf));
        CLI_LIB_PrintStr_1("IPv6 default gateway %s\r\n", tmp_buf);
/*                    CLI_LIB_PrintStr_4("\n IPv6 default gateway  %02x%02x:%02x%02x:",
                                default_gateway.addr[0], default_gateway.addr[1],
                                default_gateway.addr[2], default_gateway.addr[3]);
                    CLI_LIB_PrintStr_4("%02x%02x:%02x%02x:",
                                default_gateway.addr[4], default_gateway.addr[5],
                                default_gateway.addr[6], default_gateway.addr[7]);
                   CLI_LIB_PrintStr_4("%02x%02x:%02x%02x:",
                                default_gateway.addr[8], default_gateway.addr[9],
                                default_gateway.addr[10], default_gateway.addr[11]);
                    CLI_LIB_PrintStr_4("%02x%02x:%02x%02x\n",
                                default_gateway.addr[12], default_gateway.addr[13],
                                default_gateway.addr[14], default_gateway.addr[15]);
*/

//      CLI_LIB_PrintStr_4("IP default gateway  %d.%d.%d.%d\r\n", default_gateway[0], default_gateway[1], default_gateway[2], default_gateway[3] );
   }

//#endif

    return (UI32_T)CLI_NO_ERROR;
}
#if 0
static const UI8_T *CLI_API_IPv6_RouteTypeStr(NSM_MGR_RouteType_T type)
{
    switch (type)
    {
        case NSM_MGR_ROUTE_TYPE_IPV6_LOCAL:
            return (UI8_T *)"connected";
        case NSM_MGR_ROUTE_TYPE_IPV6_STATIC:
            return (UI8_T *)"static";
        case NSM_MGR_ROUTE_TYPE_IPV6_RIP:
            return (UI8_T *)"rip";
        case NSM_MGR_ROUTE_TYPE_IPV6_OSPF:
            return (UI8_T *)"ospf";
        case NSM_MGR_ROUTE_TYPE_IPV6_BGP:
            return (UI8_T *)"bgp";
        case NSM_MGR_ROUTE_TYPE_IPV6_ISIS:
            return (UI8_T *)"isis";
        default:
            return (UI8_T *)"unknown";
    }

    return (UI8_T *)"unknown";
}


static const UI8_T *CLI_API_IPv6_RouteSubTypeStr(NSM_MGR_RouteSubType_T type)
{
    switch (type)
    {
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_IA:
            return (UI8_T *)"IA";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_NSSA_1:
            return (UI8_T *)"N1";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_NSSA_2:
            return (UI8_T *)"N2";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_EXTERNAL_1:
            return (UI8_T *)"E1";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_EXTERNAL_2:
            return (UI8_T *)"E2";
        case NSM_MGR_ROUTE_SUBTYPE_ISIS_L1:
            return (UI8_T *)"L1";
        case NSM_MGR_ROUTE_SUBTYPE_ISIS_L2:
            return (UI8_T *)"L2";
        case NSM_MGR_ROUTE_SUBTYPE_ISIS_IA:
            return (UI8_T *)"ia";
        case NSM_MGR_ROUTE_SUBTYPE_BGP_MPLS:
        default:
            return (UI8_T *)"  ";
    }

    return (UI8_T *)"  ";
}

#endif

#if (SYS_CPNT_IPV6_ROUTING == TRUE)
static const UI8_T *CLI_API_IPv6_RouteTypeChar(NSM_MGR_RouteType_T type)
{
    switch (type)
    {
        case NSM_MGR_ROUTE_TYPE_IPV6_LOCAL:
            return (UI8_T *)"C";
        case NSM_MGR_ROUTE_TYPE_IPV6_STATIC:
            return (UI8_T *)"S";
        case NSM_MGR_ROUTE_TYPE_IPV6_RIP:
            return (UI8_T *)"R";
        case NSM_MGR_ROUTE_TYPE_IPV6_OSPF:
            return (UI8_T *)"O";
        case NSM_MGR_ROUTE_TYPE_IPV6_BGP:
            return (UI8_T *)"B";
        case NSM_MGR_ROUTE_TYPE_IPV6_ISIS:
            return (UI8_T *)"i";
        default:
            return (UI8_T *)"?";
    }

    return (UI8_T *)"?";
}

static const UI8_T *CLI_API_IPv6_RouteSubTypeChar(NSM_MGR_RouteSubType_T type)
{
    switch (type)
    {
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_IA:
            return (UI8_T *)"IA";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_NSSA_1:
            return (UI8_T *)"N1";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_NSSA_2:
            return (UI8_T *)"N2";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_EXTERNAL_1:
            return (UI8_T *)"E1";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_EXTERNAL_2:
            return (UI8_T *)"E2";
        case NSM_MGR_ROUTE_SUBTYPE_ISIS_L1:
            return (UI8_T *)"L1";
        case NSM_MGR_ROUTE_SUBTYPE_ISIS_L2:
            return (UI8_T *)"L2";
        case NSM_MGR_ROUTE_SUBTYPE_ISIS_IA:
            return (UI8_T *)"ia";
        case NSM_MGR_ROUTE_SUBTYPE_BGP_MPLS:
        default:
            return (UI8_T *)"  ";
    }

    return (UI8_T *)"    ";
}
#if 0
static void CLI_API_Show_IPv6_Route_Summary(void)
{
    NSM_MGR_RouteType_T type;
    UI32_T total = 0;
    UI32_T number;
    UI32_T fib_number = 0;
    UI8_T  multipath_num = 0;
    BOOL_T show_message = TRUE;
    // UI32_T action_flags = L_INET_ADDR_TYPE_IPV6;

    for (type = NSM_MGR_ROUTE_TYPE_IPV6_LOCAL; type < NSM_MGR_ROUTE_TYPE_IPV6_DYNAMIC; type++)
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

            CLI_LIB_PrintStr_2("%-15s %ld\n", CLI_API_IPv6_RouteTypeStr(type), number);
            total += number;
        }
    }

    CLI_LIB_PrintStr_2("%-15s %ld\n", "Total",total);
    CLI_LIB_PrintStr_2("%-15s %ld\n", "FIB", fib_number);

    return;
}


static void CLI_API_Show_IPv6_Route_Detail(NSM_MGR_RouteType_T type, BOOL_T database)
{
    NSM_MGR_GetNextRouteEntry_T entry;
    UI8_T tmp_str[CLI_DEF_MAX_BUFSIZE];
    UI8_T zero_bytes[SYS_ADPT_IPV6_ADDR_LEN] = {0};
    int len = 0;
    // UI8_T  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    // UI32_T line_num = 0;
    BOOL_T first;
    BOOL_T candidate_default;
    UI32_T ret;

    memset(&entry, 0, sizeof(NSM_MGR_GetNextRouteEntry_T));
    for(ret=NSM_PMGR_GetNextIpv6Route(&entry);TRUE;ret=NSM_PMGR_GetNextIpv6Route(&entry))
    {
        UI8_T idx;

        /* Filter type */
        if (type != NSM_MGR_ROUTE_TYPE_MAX)
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
                    if ((memcmp(entry.data.ip_route_dest.addr, zero_bytes, SYS_ADPT_IPV6_ADDR_LEN)==0) &&
                            (entry.data.ip_route_dest_prefix_len == 0) &&
                            (database == FALSE))
                    {
                        candidate_default = TRUE;
                    }

                    CLI_LIB_PrintStr_3("%s%c%s ",
                                    CLI_API_IPv6_RouteTypeChar(entry.data.ip_route_type),
                                    candidate_default ? '*' : ' ',
                                    CLI_API_IPv6_RouteSubTypeStr(entry.data.ip_route_subtype));
                    len = SYSFUN_Sprintf((char *)tmp_str, "%s%c%s ",
                                    CLI_API_IPv6_RouteTypeChar(entry.data.ip_route_type),
                                    candidate_default ? '*' : ' ',
                                    CLI_API_IPv6_RouteSubTypeStr(entry.data.ip_route_subtype));

                    if (database == TRUE)
                    {
                        CLI_LIB_PrintStr_3("%c%c%c",
                            (entry.data.next_hop_flags[idx] & NSM_TYPE_NEXTHOP_FLAG_FIB)
                            ? '*' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_SELECTED)
                            ? '>' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_STALE)
                            ? 'p' : ' ');
                        len += SYSFUN_Sprintf((char *)tmp_str, "%c%c%c",
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
                    CLI_LIB_PrintStr_4("%02x%02x:%02x%02x:",
                                entry.data.ip_route_dest.addr[0], entry.data.ip_route_dest.addr[1],
                                entry.data.ip_route_dest.addr[2], entry.data.ip_route_dest.addr[3]);
                    len += SYSFUN_Sprintf((char *)tmp_str, "%02x%02x:%02x%02x:",
                                entry.data.ip_route_dest.addr[0], entry.data.ip_route_dest.addr[1],
                                entry.data.ip_route_dest.addr[2], entry.data.ip_route_dest.addr[3]);
                     CLI_LIB_PrintStr_4("%02x%02x:%02x%02x:",
                                entry.data.ip_route_dest.addr[4], entry.data.ip_route_dest.addr[5],
                                entry.data.ip_route_dest.addr[6], entry.data.ip_route_dest.addr[7]);
                    len += SYSFUN_Sprintf((char *)tmp_str, "%02x%02x:%02x%02x:",
                                entry.data.ip_route_dest.addr[4], entry.data.ip_route_dest.addr[5],
                                entry.data.ip_route_dest.addr[6], entry.data.ip_route_dest.addr[7]);
                    CLI_LIB_PrintStr_4("%02x%02x:%02x%02x:",
                                entry.data.ip_route_dest.addr[8], entry.data.ip_route_dest.addr[9],
                                entry.data.ip_route_dest.addr[10], entry.data.ip_route_dest.addr[11]);
                    len += SYSFUN_Sprintf((char *)tmp_str, "%02x%02x:%02x%02x:",
                                entry.data.ip_route_dest.addr[8], entry.data.ip_route_dest.addr[9],
                                entry.data.ip_route_dest.addr[10], entry.data.ip_route_dest.addr[11]);
                     CLI_LIB_PrintStr_4("%02x%02x:%02x%02x/",
                                entry.data.ip_route_dest.addr[12], entry.data.ip_route_dest.addr[13],
                                entry.data.ip_route_dest.addr[14], entry.data.ip_route_dest.addr[15]);
                    len += SYSFUN_Sprintf((char *)tmp_str, "%02x%02x:%02x%02x/",
                                entry.data.ip_route_dest.addr[12], entry.data.ip_route_dest.addr[13],
                                entry.data.ip_route_dest.addr[14], entry.data.ip_route_dest.addr[15]);

                    CLI_LIB_PrintStr_1("%d", entry.data.ip_route_dest_prefix_len);
                    len += SYSFUN_Sprintf((char *)tmp_str, "%d", entry.data.ip_route_dest_prefix_len);

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

                    SYSFUN_Sprintf((char *)tmp_str, "%*c", len - 8, ' ');
                    CLI_LIB_PrintStr((char *)tmp_str);
                }

                if (entry.data.ip_route_type != NSM_MGR_ROUTE_TYPE_IPV6_LOCAL)
                    CLI_LIB_PrintStr_2("[%d/%ld]", entry.data.distance, (long)entry.data.metric);

                switch (entry.data.next_hop_type[idx])
                {
                    case NSM_TYPE_NEXTHOP_TYPE_IPV6:
                    case NSM_TYPE_NEXTHOP_TYPE_IPV6_IFNAME:
                    case NSM_TYPE_NEXTHOP_TYPE_IPV6_IFINDEX:
//                        CLI_LIB_PrintStr_4(" via %d.%d.%d.%d",
//                                entry.data.ip_next_hop[idx][0], entry.data.ip_next_hop[idx][1],
//                                entry.data.ip_next_hop[idx][2], entry.data.ip_next_hop[idx][3]);
                    CLI_LIB_PrintStr_4("\n via %02x%02x:%02x%02x:",
                                entry.data.ip_next_hop[idx].addr[0], entry.data.ip_next_hop[idx].addr[1],
                                entry.data.ip_next_hop[idx].addr[2], entry.data.ip_next_hop[idx].addr[3]);
                    CLI_LIB_PrintStr_4("%02x%02x:%02x%02x:",
                                entry.data.ip_next_hop[idx].addr[4], entry.data.ip_next_hop[idx].addr[5],
                                entry.data.ip_next_hop[idx].addr[6], entry.data.ip_next_hop[idx].addr[7]);
                   CLI_LIB_PrintStr_4("%02x%02x:%02x%02x:",
                                entry.data.ip_next_hop[idx].addr[8], entry.data.ip_next_hop[idx].addr[9],
                                entry.data.ip_next_hop[idx].addr[10], entry.data.ip_next_hop[idx].addr[11]);
                    CLI_LIB_PrintStr_4("%02x%02x:%02x%02x",
                                entry.data.ip_next_hop[idx].addr[12], entry.data.ip_next_hop[idx].addr[13],
                                entry.data.ip_next_hop[idx].addr[14], entry.data.ip_next_hop[idx].addr[15]);

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
        // PROCESS_MORE(buff);
    }

    return;
}
#endif

static UI32_T CLI_API_Show_IPv6_Route_More(NSM_MGR_RouteType_T type , L_PREFIX_T *prefix, L_INET_AddrIp_T * addr, char* interface_name, BOOL_T database, UI32_T *line_num_p)

{
    NSM_MGR_GetNextRouteEntry_T entry;
    char  tmp_str[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
    UI8_T zero_bytes[SYS_ADPT_IPV6_ADDR_LEN] = {0};

    BOOL_T first;
    BOOL_T candidate_default;
    UI32_T ret;
    UI32_T line_num = 0;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};

    line_num = *line_num_p;
     
    INFOprintf("type=%d, is null? prefix=%s, addr=%s,if=%s",type, prefix ==NULL?"Y":"N", addr==NULL?"Y":"N",interface_name==NULL?"Y":"N");
    if(addr!=NULL)
        INFOprintf("inpute %lx:%lx:%lx:%lx/%d, len=%d, type=%d",
                            L_INET_EXPAND_IPV6(addr->addr),
                            addr->preflen,
                            addr->addrlen,
                            addr->type);

    if(interface_name!=NULL)
        INFOprintf("inpute name=%s", interface_name);
    memset(&entry, 0, sizeof(NSM_MGR_GetNextRouteEntry_T));
    for(ret=NSM_PMGR_GetNextIpv6Route(&entry);TRUE;ret=NSM_PMGR_GetNextIpv6Route(&entry))
    {
        UI8_T idx;

        if(database == FALSE && !(entry.data.flags & NSM_TYPE_ROUTE_SELECTED))
        {
            if(ret == NSM_TYPE_RESULT_OK)
                continue;
            else
                break;
        }
        
        /* Filter type */
        if (type != NSM_MGR_ROUTE_TYPE_MAX)
        {
            if (entry.data.ip_route_type != type)
            {
                if (ret == NSM_TYPE_RESULT_OK)
                    continue;
                break;
            }
        }
        /* Filter prefix */
        if(prefix != NULL)
        {
            L_PREFIX_T destination;
            L_PREFIX_Inet6Addr2Prefix(entry.data.ip_route_dest.addr, entry.data.ip_route_dest_prefix_len, &destination);
            L_PREFIX_ApplyMask(&destination);
            L_PREFIX_ApplyMask(prefix);
            if(memcmp(&destination.u.prefix ,&prefix->u.prefix,SYS_ADPT_IPV6_ADDR_LEN)!=0)
            {
                if(ret == NSM_TYPE_RESULT_OK)
                    continue;
                break;
            }
        }

        /* Default route check.  */
        if (FALSE == database
            && entry.data.ip_route_dest_prefix_len == 0
            && !memcmp(entry.data.ip_route_dest.addr, zero_bytes, SYS_ADPT_IPV6_ADDR_LEN))
            candidate_default = TRUE;
        else
            candidate_default = FALSE;


        if (ret == NSM_TYPE_RESULT_OK || ret == NSM_TYPE_RESULT_EOF)
        {
            first = TRUE;

            for(idx=0; idx<entry.data.num_of_next_hop; idx++)
            {
                INFOprintf("Dump Rif %lx:%lx:%lx:%lx,,strlen=%d, p=%d, type=%d,len=%d,pre=%d, iftype=%d, if=%ld, name=%s",
                                L_INET_EXPAND_IPV6(entry.data.ip_next_hop[idx].addr),
                                entry.data.ip_next_hop[idx].addrlen,
                                entry.data.ip_next_hop[idx].preflen,
                                entry.data.ip_next_hop[idx].type,
                                entry.data.ip_next_hop[idx].addrlen,
                                entry.data.ip_next_hop[idx].preflen,
                                entry.data.next_hop_type[idx],
                                (long)entry.data.next_hop_ifindex[idx],
                                entry.data.next_hop_ifname[idx]);

                /* Filter non-FIB route or Inactive route */
                if(database == FALSE && !(entry.data.next_hop_flags[idx] & NSM_TYPE_NEXTHOP_FLAG_FIB))
                {
                    continue;
                }

                /* Filter if name */
                if(interface_name!=NULL)
                {
                    if(strcmp(interface_name, entry.data.next_hop_ifname[idx])!=0)
                    {
                        if (ret == NSM_TYPE_RESULT_OK)
                            continue;
                        break;
                    }
                }
                /* Filter dest addr */
                if(addr!=NULL)
                {
                    if(memcmp(entry.data.ip_next_hop[idx].addr, addr->addr, SYS_ADPT_IPV6_ADDR_LEN) !=0)
                    {
                        INFOprintf("len=%d-%d, pre=%d-%d", entry.data.ip_next_hop[idx].addrlen, addr->addrlen, entry.data.ip_next_hop[idx].preflen, addr->addrlen);
                        if (ret == NSM_TYPE_RESULT_OK)
                            continue;
                        break;
                    }
                }
                if(entry.data.num_of_next_hop > 1)
                {
                    if(first)
                    {
                        /* len is 7 */
                        snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), "%s%c%s   ",
                            CLI_API_IPv6_RouteTypeChar(entry.data.ip_route_type),
                            candidate_default ? '*' : ' ',
                            CLI_API_IPv6_RouteSubTypeChar(entry.data.ip_route_subtype));
                        /* destination addr */
                        entry.data.ip_route_dest.preflen = entry.data.ip_route_dest_prefix_len;
                        L_INET_InprefixToString((L_INET_Addr_T *)&entry.data.ip_route_dest, sizeof(tmp_str), tmp_str);
                        snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), "%s\r\n",tmp_str);
                        PROCESS_MORE(buff);
                        first = FALSE;
                        if (database == TRUE) /* Display FIB status when database flag is specified. */
                        {
                            /*len is 9 */
                            snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), "    %c%c%c  ",
                                CHECK_FLAG (entry.data.next_hop_flags[idx], NSM_TYPE_NEXTHOP_FLAG_FIB) ? '*' : ' ',
                                CHECK_FLAG (entry.data.flags, NSM_TYPE_ROUTE_SELECTED) ? '>' : ' ',
                                CHECK_FLAG (entry.data.flags, NSM_TYPE_ROUTE_STALE) ? 'p' : ' ');
                        }
                        else
                        {   /* len is 9 */
                            snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), "         ");
                        }
                    }
                    else
                    {
                        if (database == TRUE) /* Display FIB status when database flag is specified. */
                        {
                            /*len is 9 */
                            snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), "    %c%c%c  ",
                                CHECK_FLAG (entry.data.next_hop_flags[idx], NSM_TYPE_NEXTHOP_FLAG_FIB) ? '*' : ' ',
                                CHECK_FLAG (entry.data.flags, NSM_TYPE_ROUTE_SELECTED) ? '>' : ' ',
                                CHECK_FLAG (entry.data.flags, NSM_TYPE_ROUTE_STALE) ? 'p' : ' ');
                        }
                        else
                        {   /* len is 9 */
                            snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), "         ");
                        }
                    }
                }
                else
                {
                    /* len is 4 */
                    snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), "%s%c%s",
                        CLI_API_IPv6_RouteTypeChar(entry.data.ip_route_type),
                        candidate_default ? '*' : ' ',
                        CLI_API_IPv6_RouteSubTypeChar(entry.data.ip_route_subtype));
                    if (database == TRUE) /* Display FIB status when database flag is specified. */
                    {
                        snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), "%c%c%c",
                            CHECK_FLAG (entry.data.next_hop_flags[idx], NSM_TYPE_NEXTHOP_FLAG_FIB) ? '*' : ' ',
                            CHECK_FLAG (entry.data.flags, NSM_TYPE_ROUTE_SELECTED) ? '>' : ' ',
                            CHECK_FLAG (entry.data.flags, NSM_TYPE_ROUTE_STALE) ? 'p' : ' ');
                    }
                    else
                    {
                        snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), "   ");
                    }
                    /* destination addr */
                    entry.data.ip_route_dest.preflen = entry.data.ip_route_dest_prefix_len;
                    L_INET_InprefixToString((L_INET_Addr_T *)&entry.data.ip_route_dest, sizeof(tmp_str), tmp_str);
                    snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), "%s",tmp_str);

                }

                switch (entry.data.next_hop_type[idx])
                {
                    case NSM_TYPE_NEXTHOP_TYPE_IPV6:
                    case NSM_TYPE_NEXTHOP_TYPE_IPV6_IFNAME:
                    case NSM_TYPE_NEXTHOP_TYPE_IPV6_IFINDEX:
                        /* initial tmp_str */
                        tmp_str[0] = '\0';

                        L_INET_InaddrToString((L_INET_Addr_T *)&entry.data.ip_next_hop[idx], tmp_str, sizeof(tmp_str));

                        snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), " [%d/%lu] via %s", entry.data.distance, (unsigned long)entry.data.metric, tmp_str);
                        if (entry.data.flags & NSM_TYPE_ROUTE_BLACKHOLE)
                            snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), ", Null0");
                        else if (entry.data.next_hop_ifname[idx][0] != '\0')
                        {
                            snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), ", %s", entry.data.next_hop_ifname[idx]);
                            entry.data.next_hop_ifname[idx][0] = '\0';
                        }
                        else if (entry.data.next_hop_ifindex[idx])
                        {
                            snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), ", %s", entry.data.next_hop_ifname[idx]);
                        }

                        break;

                    case NSM_TYPE_NEXTHOP_TYPE_IFINDEX:
                        INFOprintf("Name1 = %s", entry.data.next_hop_ifname[idx]);
                        snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), ", %s", entry.data.next_hop_ifname[idx]);
                        entry.data.next_hop_ifname[idx][0] = '\0';
                        break;

                    case NSM_TYPE_NEXTHOP_TYPE_IFNAME:
                        INFOprintf("Name2 = %s", entry.data.next_hop_ifname[idx]);
                        snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), ", %s", entry.data.next_hop_ifname[idx]);
                        entry.data.next_hop_ifname[idx][0] = '\0';
                        break;

                    default:
                        break;
                }
                if (! CHECK_FLAG (entry.data.next_hop_flags[idx], NSM_TYPE_NEXTHOP_FLAG_ACTIVE))
                    snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), " inactive");

                snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), "\r\n");
                PROCESS_MORE(buff);
            }
        }

        if(ret != NSM_TYPE_RESULT_OK)
            break;
    }

    *line_num_p = line_num;
    return CLI_NO_ERROR;
}
#endif

UI32_T  CLI_API_L3_Show_IpV6_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    UI32_T line_num = 0;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};

    const char show_header[] =
        "Codes: C - connected, S - static, R - RIP, B - BGP\r\n"
        "       O - OSPF, IA - OSPF inter area\r\n"
        "       N1 - OSPF NSSA external type 1, N2 - OSPF NSSA external type 2\r\n"
        "       E1 - OSPF external type 1, E2 - OSPF external type 2\r\n"
        "       i - IS-IS, L1 - IS-IS level-1, L2 - IS-IS level-2,"
        " ia - IS-IS inter area\r\n";
    /* Normal case.  */
    char header_normal[] =
        "       * - candidate default\r\n\r\n";

    /* Database case.  */
    char header_database[] =
        "       > - selected route, * - FIB route, p - stale info\r\n\r\n";

    NSM_MGR_RouteType_T type = NSM_MGR_ROUTE_TYPE_MAX;
    L_PREFIX_T prefix, *prefix_p = NULL;
    L_INET_AddrIp_T address, *address_p = NULL;
    char ifname[16] = {}, *ifname_p = NULL;
    BOOL_T database = FALSE; /* FIB (active) route or non-FIB (inactive) route */


    if(arg [0] == NULL)
    {
        type = NSM_MGR_ROUTE_TYPE_MAX;
    }
    else if (L_INET_RETURN_SUCCESS == L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC,
                                                            arg[0],
                                                            (L_INET_Addr_T *)&address,
                                                            sizeof(address)))
    {
        INFOprintf("get inpute %lx:%lx:%lx:%lx; t=%d, l=%d, p=%d",
                            L_INET_EXPAND_IPV6(address.addr),
                            address.type,
                            address.addrlen,
                            address.preflen);
        if(strstr((char*)arg[0], "/")!=NULL)
        {

            L_PREFIX_Inet6Addr2Prefix(address.addr, address.preflen, &prefix);
            INFOprintf("This is a prefix");
            prefix_p = &prefix;
        }
        else
        {
            address_p = &address;
        }
    }
    else if (arg [0][0]!='i' && arg[0][0]!='I')
    {//protocol
        switch (arg[0][0])
        {
            case 'L':
            case 'l':
                type = NSM_MGR_ROUTE_TYPE_IPV6_LOCAL;
                break;
            case 'S':
            case 's':
                type = NSM_MGR_ROUTE_TYPE_IPV6_STATIC;
                break;
            case 'R':
            case 'r':
                type = NSM_MGR_ROUTE_TYPE_IPV6_RIP;
                break;
            case 'B':
            case 'b':
                type =  NSM_MGR_ROUTE_TYPE_IPV6_BGP;
                break;
            case 'O': //OSPF
            case 'o':
                type = NSM_MGR_ROUTE_TYPE_IPV6_OSPF;
                break;
            case 'D':
            case 'd':
                /* All routes including inactive routes. (static/ospf/...) */
                database = TRUE;
                break;
        }
    }
    else
    {//interface vlan 1
        int ifid = atoi((char*)arg[2]);
        INFOprintf("ifindex=%d", ifid);
        switch (arg [1][0])
        {
            case 'V':
            case 'v':
                SYSFUN_Sprintf((char *)ifname,"VLAN%d",ifid);
                break;
            case 'T':
            case 't':
                SYSFUN_Sprintf((char *)ifname,"TUNNEL%d",ifid);
                break;
            default:
                DBGprintf("Unexpected error!");
                return (UI32_T)CLI_ERR_CMD_INVALID;
        }
        ifname_p = ifname;
    }

    snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), (char *)show_header);
    PROCESS_MORE(buff);
    if (database == TRUE)
        snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), header_database);
    else
        snprintf(buff + strlen(buff), CLI_DEF_MAX_BUFSIZE - strlen(buff), header_normal);
    PROCESS_MORE(buff);

    CLI_API_Show_IPv6_Route_More(type , prefix_p, address_p, ifname_p, database, &line_num);
#endif

    return (UI32_T)CLI_NO_ERROR;
}


#if 0
UI32_T  CLI_API_L3_Show_IpV6_Route(UI16_T cmd_idx, UI8_T *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
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
    NSM_MGR_RouteType_T type = NSM_MGR_ROUTE_TYPE_MAX;
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
                type = NSM_MGR_ROUTE_TYPE_IPV6_LOCAL;
                break;

            case 's':
            case 'S':
                if ((arg[0][1] == 't') || (arg[0][1] == 'T'))
                    type = NSM_MGR_ROUTE_TYPE_IPV6_STATIC;
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
        CLI_API_Show_IPv6_Route_Summary();
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr((char *)show_header);
    if (database == TRUE)
        CLI_LIB_PrintStr((char *)header_database);
    else
        CLI_LIB_PrintStr((char *)header_normal);

    CLI_API_Show_IPv6_Route_Detail(type, database);

    return (UI32_T)CLI_NO_ERROR;
}
#endif
#if 0
UI32_T CLI_API_IPv6_UnicastRouting(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
    UI32_T ret, vr_id = 0;
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_IPV6_UNICASTROUTING :
            ret = NETCFG_PMGR_ROUTE_EnableIpForwarding(vr_id, L_INET_ADDR_TYPE_IPV6);

            switch(ret)
            {
                case NETCFG_TYPE_NO_CHANGE:
                case NETCFG_TYPE_OK:
                    break;
                default:
                    CLI_LIB_PrintStr("Failed to enable IPv6 unicast-routing.\r\n");
                    break;
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IPV6_UNICASTROUTING :
            ret = NETCFG_PMGR_ROUTE_DisableIpForwarding(vr_id, L_INET_ADDR_TYPE_IPV6);
            switch(ret)
            {
                case NETCFG_TYPE_NO_CHANGE:
                case NETCFG_TYPE_OK:
                    break;
                default:
                    CLI_LIB_PrintStr("Failed to disable IPv6 unicast-routing.\r\n");
                break;
            }
            break;

        default:
            break;
    }
#endif /* SYS_CPNT_IPV6 */
    return (UI32_T)CLI_NO_ERROR;
}
#endif

//UI32_T  CLI_API_Ipv6_Default_Gateway(UI16_T cmd_idx, UI8_T *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
//{
//    /* Not implemented */
//    return (UI32_T)CLI_NO_ERROR;
//}
//
//
//UI32_T  CLI_API_L3_IPv6_Route(UI16_T cmd_idx, UI8_T *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
//{
//    /* Not implemented */
//    return (UI32_T)CLI_NO_ERROR;
//}
//UI32_T  CLI_API_Show_Ipv6_Default_Gateway(UI16_T cmd_idx, UI8_T *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
//{
//    /* Not implemented */
//    return (UI32_T)CLI_NO_ERROR;
//}
//
//
//
//UI32_T  CLI_API_L3_Show_IpV6_Route(UI16_T cmd_idx, UI8_T *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
//{
//    /* Not implemented */
//    return (UI32_T)CLI_NO_ERROR;
//}
//
//

/* refer to cli_api_system.c, share with CLI_API_TraceRoute
 */
extern char *cli_api_get_tracert_code_tag(UI32_T code);
extern char *cli_api_get_tracert_code_str(UI32_T code);
extern char *cli_api_get_tracert_status_str(
                char    *buff_p,
                UI32_T  *code_bmp_p,
                UI32_T  code,
                UI32_T  resp);
extern void cli_api_show_tracert_code_bmp(UI32_T code_bmp);

UI32_T CLI_API_IPv6_TraceRoute(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  owner_index_len = 0;
    UI32_T  test_name_len = 0;
    UI32_T  prev_hop = 0, ret = 0;
    UI32_T  i = 0, code_bmp = 0;
    UI16_T  key_ret = 0;
    char    owner_index[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1] = {0};
    char    test_name[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1] = {0};
    char    trace_status[22] = {0};
    TRACEROUTE_TYPE_TraceRouteCtlEntry_T          ctrl_entry;
    TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T prob_history_entry;
    L_INET_AddrIp_T inet_target_addr;
    char    ip_str[L_INET_MAX_IP6ADDR_STR_LEN+1] = {0};
    char    buff[CLI_DEF_MAX_BUFSIZE];
    BOOL_T  has_ip = FALSE;
    UI32_T  start_time;
    UI32_T  max_failures;
    UI32_T  ctl_timeout, ctl_probes_per_hop;

    ctl_timeout = SYS_DFLT_TRACEROUTE_CTL_TIME_OUT; /* in system tick unit */
    ctl_probes_per_hop = SYS_DFLT_TRACEROUTE_CTL_PROBES_PER_HOP;

    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;

    if (L_INET_RETURN_SUCCESS == L_INET_StringToInaddr(L_INET_FORMAT_IPV4_UNSPEC,
                                                       arg[0],
                                                       (L_INET_Addr_T *) &inet_target_addr,
                                                       sizeof(inet_target_addr))
        ||
        L_INET_RETURN_SUCCESS == L_INET_StringToInaddr(L_INET_FORMAT_IPV6_PREFIX,
                                                       arg[0],
                                                       (L_INET_Addr_T *) &inet_target_addr,
                                                       sizeof(inet_target_addr))
        )
    {
        CLI_LIB_PrintStr_1("Invalid IPv6 address - %s\r\n", arg[0]);
        return CLI_NO_ERROR;
    }

    /* max-failures, use sys-default or user-specified value.
     * 0 or 255 means to disable this feature (rfc 2925).
     * To avoid confusion, 0 is not allowed in CLI.
     */
    max_failures = SYS_DFLT_TRACEROUTE_CTL_MAX_FAILURE;

    if(arg[1] && (arg[1][0] == 'm'))
    {
        /* assume arg[2] exists and numeric */
        max_failures = atoi(arg[2]);
    }

    memset(test_name, 0, sizeof(test_name));
    memset(owner_index, 0, sizeof(owner_index));
    memset(&ctrl_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));

    CLI_LIB_PrintStr("Press \"ESC\" to abort.\r\n");
    CLI_LIB_PrintStr_1("Traceroute to %s", arg[0]);

    strncpy(owner_index,
        TRACEROUTE_TYPE_CTL_OWNER_INDEX_SYSTEM_CLI,
        sizeof(owner_index) - 1);
    owner_index[sizeof(owner_index) - 1] = '\0';
    owner_index_len = strlen(owner_index);

    snprintf((char *)test_name, sizeof(test_name),
        "%d", CLI_TASK_GetMySessId());
    test_name[sizeof(test_name) - 1] = '\0';
    test_name_len = strlen(test_name);

    start_time = SYSFUN_GetSysTick();

    memset(&inet_target_addr, 0, sizeof(inet_target_addr));

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC,
                                                       arg[0],
                                                       (L_INET_Addr_T *)&inet_target_addr,
                                                       sizeof(inet_target_addr)))
    {
#if (SYS_CPNT_DNS == TRUE)
        DNS_Nslookup_CTRL_T nslookup_ctl_entry;
        DNS_Nslookup_Result_T nslookup_result_entry;
        UI32_T nslookup_ctl_index;
        UI32_T nslookup_result_index;

        memset(&nslookup_ctl_entry, 0, sizeof(nslookup_ctl_entry));

        strncpy((char *)nslookup_ctl_entry.CtlOwnerIndex,
            DNS_TYPE_NSLOOKUP_CTL_OWNER_INDEX_SYSTEM_CLI,
            sizeof(nslookup_ctl_entry.CtlOwnerIndex) - 1);
        nslookup_ctl_entry.CtlOwnerIndex[sizeof(nslookup_ctl_entry.CtlOwnerIndex) - 1] = '\0';
        nslookup_ctl_entry.CtlOwnerIndexLen = strlen((char *)nslookup_ctl_entry.CtlOwnerIndex);

        snprintf((char *)nslookup_ctl_entry.OperationName, sizeof(nslookup_ctl_entry.OperationName),
            "%d", CLI_TASK_GetMySessId());
        nslookup_ctl_entry.OperationName[sizeof(nslookup_ctl_entry.OperationName) - 1] = '\0';
        nslookup_ctl_entry.OperationNameLen = strlen((char *)nslookup_ctl_entry.OperationName);

        strncpy((char *)nslookup_ctl_entry.TargetAddress,
            arg[0],
            sizeof(nslookup_ctl_entry.TargetAddress)-1);
        nslookup_ctl_entry.TargetAddress[sizeof(nslookup_ctl_entry.TargetAddress) - 1] = '\0';
        nslookup_ctl_entry.TargetAddressLen = strlen((char *)nslookup_ctl_entry.TargetAddress);

        nslookup_ctl_entry.af_family = AF_INET6;

        if (DNS_OK != DNS_PMGR_CreateSystemNslookupCtlEntry(&nslookup_ctl_entry, &nslookup_ctl_index))
        {
            CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
            return CLI_NO_ERROR;
        }

        while (1)
        {
            if (CLI_IO_ReadACharNoWait() == 0x1B) /*ESC*/
            {
                UI8_T tmp_char;
                /* EPR ID:ES4626F-SW-FLF-38-00595
                 * Arrow keys are consisted by 3 characters
                 * and the 1st character is 0x1B
                 * Users are not happy to see the latter 2 characters being
                 * printed. So do the workaround here to remove these 2 characters
                 * from Telnet rx buffer to avoid being printed by CLI latter
                 * Cross reference to CLI_IO_GetKeyByVtTerminal() in cli_io.c
                 */

                tmp_char = CLI_IO_ReadACharNoWait(ctrl_P);
                if (tmp_char!=0)
                {
                    if (tmp_char==0x5B || tmp_char==0x4F) /* 2nd char of arrow key */
                    {
                        /* read the last char to clear arrow key in buffer
                         */
                        CLI_IO_ReadACharNoWait(ctrl_P);
                    }
                }

                /* silently destroy ctl entry
                 */
                CLI_LIB_Printf("\r\n");
                DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                return CLI_NO_ERROR;
            }

            memset(&nslookup_ctl_entry, 0, sizeof(nslookup_ctl_entry));

            if (DNS_OK == DNS_PMGR_GetNslookupCtlEntryByIndex(nslookup_ctl_index, &nslookup_ctl_entry))
            {
                if (NSLOOKUP_OPSTATUS_COMPLETED == nslookup_ctl_entry.OperStatus)
                {
                    if (DNS_OK == nslookup_ctl_entry.Rc)
                    {
                        nslookup_result_index = 0;
                        memset(&nslookup_result_entry, 0, sizeof(nslookup_result_entry));

                        if (DNS_OK == DNS_PMGR_GetNslookupResultEntryByIndex(
                            nslookup_ctl_index, nslookup_result_index, &nslookup_result_entry))
                        {
                            memcpy(&inet_target_addr, &nslookup_result_entry.ResultsAddress_str, sizeof(inet_target_addr));
                            DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                            break;
                        }
                    }

                    CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
                    DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                    return CLI_NO_ERROR;
                }
            }
            else
            {
                CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
                return CLI_NO_ERROR;
            }

            /* To avoid somthing wrong, CLI maybe halt, we wait a bit longer than total timeout, then break */
            if((SYSFUN_GetSysTick() - start_time) > ctl_timeout * (ctl_probes_per_hop + 1))
            {
                CLI_LIB_PrintStr_1("\r\nTimeout for lookup host name %s.\r\n", arg[0]);
                DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                return CLI_NO_ERROR;
            }

            SYSFUN_Sleep(50);
        }
        /* display ipv6 address got from DNS */
        L_INET_InaddrToString((L_INET_Addr_T *) &inet_target_addr, ip_str, sizeof(ip_str));
        CLI_LIB_PrintStr_1(" [%s]", ip_str);
#else
        /* No DNS
         */
        CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
        return CLI_NO_ERROR;
#endif
    }

    /* 1. link-local address must has zone-id (%N), which maybe 0, thus the string is checked. */
    /* check FE80::/10 by low-level function */

    if(L_INET_ADDR_IS_IPV6_LINK_LOCAL(inet_target_addr.addr))
    {
        if(strchr((char*)arg[0], '%') == NULL)
        {
            CLI_LIB_Printf("\r\nInvalid address format.\r\n");
            CLI_LIB_Printf("Please specify a ZoneID(%%N) for link-local address.\r\n");
            return CLI_NO_ERROR;
        }
    }
    /* 2. reject multicast address */
    else if (inet_target_addr.addr[0] == 0xff)
    {
        CLI_LIB_Printf("\r\nInvalid address format.\r\n");
        CLI_LIB_PrintStr("IPv6 multicast address is not allowed.\r\n");
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr_2(", %d hops max, timeout is %d seconds, ",
        SYS_DFLT_TRACEROUTE_CTL_MAX_TTL,
        SYS_DFLT_TRACEROUTE_CTL_TIME_OUT/SYS_BLD_TICKS_PER_SECOND);

    CLI_LIB_PrintStr_1("%ld max failure(s) before termination.\r\n\r\n", (long)max_failures);

    strncpy(ctrl_entry.trace_route_ctl_owner_index, owner_index, sizeof(ctrl_entry.trace_route_ctl_owner_index) - 1);
    ctrl_entry.trace_route_ctl_owner_index[sizeof(ctrl_entry.trace_route_ctl_owner_index) - 1] = '\0';
    ctrl_entry.trace_route_ctl_owner_index_len = strlen(ctrl_entry.trace_route_ctl_owner_index);
    strncpy(ctrl_entry.trace_route_ctl_test_name, test_name, sizeof(ctrl_entry.trace_route_ctl_test_name) - 1);
    ctrl_entry.trace_route_ctl_test_name[sizeof(ctrl_entry.trace_route_ctl_test_name) - 1] = '\0';
    ctrl_entry.trace_route_ctl_test_name_len = strlen(ctrl_entry.trace_route_ctl_test_name);
    ctrl_entry.trace_route_ctl_max_failures = max_failures;

    ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_createAndGo;

    if (TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry) != TRACEROUTE_TYPE_OK)
    {
        CLI_LIB_PrintStr("Failed to start traceroute6.\r\n");
        return CLI_NO_ERROR;
    }

    /* target address type */
    ctrl_entry.trace_route_ctl_target_address_type = inet_target_addr.type;
    ret = TRACEROUTE_PMGR_SetCtlEntryByField(&ctrl_entry, TRACEROUTE_TYPE_CTLENTRYFIELD_TARGET_ADDRESS_TYPE);

    if(TRACEROUTE_TYPE_OK != ret)
    {
        CLI_LIB_PrintStr("\r\nInvalid address type.\r\n");
        /* silently destroy ctl entry */
        ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
        TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry);
        return CLI_NO_ERROR;
    }

    /* target address */
    ctrl_entry.trace_route_ctl_target_address = inet_target_addr;
    ret = TRACEROUTE_PMGR_SetCtlEntryByField(&ctrl_entry, TRACEROUTE_TYPE_CTLENTRYFIELD_TARGET_ADDRESS);
    if(TRACEROUTE_TYPE_OK != ret)
    {
        CLI_LIB_PrintStr("\r\nInvalid address.\r\n");
        /* silently destroy ctl entry */
        ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
        TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry);
        return CLI_NO_ERROR;
    }

    ctrl_entry.trace_route_ctl_admin_status = VAL_traceRouteCtlAdminStatus_enabled;

    if (TRACEROUTE_TYPE_OK != TRACEROUTE_PMGR_SetTraceRouteCtlAdminStatus(&ctrl_entry))
    {
        CLI_LIB_PrintStr("Failed to enable traceroute6.\r\n");
        ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
        if (TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry) != TRACEROUTE_TYPE_OK)
        {
            CLI_LIB_PrintStr("Failed to terminate traceroute.\r\n");
        }
        return CLI_NO_ERROR;
    }

    /* Sample output:
     *
     *  Press "ESC" to abort.
     *  Traceroute to 2001::1, 30 hops max, timeout is 3 seconds
     *
     *  Hop Packet 1 Packet 2 Packet 3 IPv6 Address
     *  --- -------- -------- -------- --------------------------------------------
     *    1        *        *        *
     *    2        *        *        *
     *  Codes:
     *    * - No Response
     */

    memset(&prob_history_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));

    snprintf(buff, sizeof(buff), "%-3s %-8s %-8s %-8s %-44s\r\n",
        "Hop", "Packet 1", "Packet 2", "Packet 3", "IPv6 Address");
    CLI_LIB_PrintStr(buff);

    snprintf(buff, sizeof(buff), "%-3s %-8s %-8s %-8s %44s\r\n",
        "---", "--------", "--------", "--------", "--------------------------------------------");
    CLI_LIB_PrintStr(buff);

    prev_hop = 0;
    while (1)
    {
        key_ret = 0;
        if(ctrl_P->sess_type == CLI_TYPE_TELNET)
        {
            if( CLI_IO_ReadACharFromTelnet(ctrl_P) == 0x1B) //ESC
                key_ret = TRUE;
        }
        else
        {
            if( CLI_IO_ReadACharFromConsole(ctrl_P) == 0x1B) //ESC
                key_ret = TRUE;
        }

        if (key_ret)
        {
            ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
            if (TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry) != TRACEROUTE_TYPE_OK)
            {
                CLI_LIB_PrintStr("Failed to terminate traceroute6.\r\n");
            }
            else
            {
                CLI_LIB_PrintStr("\r\nTrace stopped.\r\n");
            }
            return CLI_NO_ERROR;
        }
        ret = TRACEROUTE_POM_GetNextTraceRouteProbeHistoryEntryForCLIWEB(
                  owner_index, owner_index_len, test_name, test_name_len,
                  prob_history_entry.trace_route_probe_history_index,
                  prob_history_entry.trace_route_probe_history_hop_index,
                  prob_history_entry.trace_route_probe_history_probe_index,
                  &prob_history_entry);

        if ((ret == TRACEROUTE_TYPE_FAIL) || (ret == TRACEROUTE_TYPE_NO_MORE_ENTRY))
        {
            cli_api_show_tracert_code_bmp(code_bmp);
            CLI_LIB_PrintStr("\r\nTrace completed.\r\n");
            break;
        }
        else if (ret == TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE)
        {
            SYSFUN_Sleep(10);
            continue;
        }

        if (prob_history_entry.trace_route_probe_history_hop_index == 0)
            continue;

        if(prev_hop != prob_history_entry.trace_route_probe_history_hop_index)
        {
            if (i != 3 && i != 0)
            {
                CLI_LIB_PrintStr("\r\n");
                i = 0;
            }

            has_ip = FALSE;
            prev_hop = prob_history_entry.trace_route_probe_history_hop_index;
            CLI_LIB_PrintStr_1("%3d ", (int)prob_history_entry.trace_route_probe_history_hop_index);
        }

        if (FALSE == has_ip)
        {
            if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&prob_history_entry.trace_route_probe_history_haddr,
                                                               ip_str,
                                                               sizeof(ip_str)))
            {
                has_ip = TRUE;
            }
        }

        CLI_LIB_PrintStr_1("%8s ",
            cli_api_get_tracert_status_str(
                trace_status,
                &code_bmp,
                prob_history_entry.trace_route_probe_history_last_rc,
                prob_history_entry.trace_route_probe_history_response));

        if (++i == 3)
        {
            if (TRUE == has_ip)
            {
                CLI_LIB_PrintStr_1("%s", ip_str);
            }

            if(!memcmp(&prob_history_entry.trace_route_probe_history_haddr, &inet_target_addr, sizeof(L_INET_AddrIp_T)))
            {
                CLI_LIB_PrintStr("\r\n");

                cli_api_show_tracert_code_bmp(code_bmp);

                ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;

                if (TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry) != TRACEROUTE_TYPE_OK)
                {
                    CLI_LIB_PrintStr("Failed to terminate traceroute6.\r\n");
                }
                else
                {
                    CLI_LIB_PrintStr("\r\nTrace completed.\r\n");
                }
                return CLI_NO_ERROR;
            }
            CLI_LIB_PrintStr("\r\n");
            i = 0;
        }
    } // end of while

    ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;

    if (TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry) != TRACEROUTE_TYPE_OK)
    {
        CLI_LIB_PrintStr("Failed to terminate traceroute6.\r\n");
    }
    return CLI_NO_ERROR;
}


#if (SYS_CPNT_CRAFT_PORT == TRUE)
UI32_T CLI_API_Craft_Interface_Ipv6_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T ipv6_addr_type;
    UI32_T rc_netcfg;

    NETCFG_TYPE_CraftInetAddress_T craft_addr;

    ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL;
    memset(&craft_addr, 0, sizeof(craft_addr));

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_CRAFT_CMD_W2_IPV6_ADDRESS:

            if(arg[0] == NULL)
                return CLI_ERR_INTERNAL;

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_ADDR_RAW,
                                                               arg[0],
                                                               (L_INET_Addr_T *)&craft_addr.addr,
                                                               sizeof(craft_addr.addr)))
            {
                CLI_LIB_PrintStr("Invalid IPv6 address format.\r\n");
                return CLI_NO_ERROR;
            }

            if(arg[1] == NULL)
            {

                /* check link-local address */
                if(L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL(craft_addr.addr.addr))
                {
                    CLI_LIB_PrintStr("Use ""ipv6 address X:X:X:X::X link-local"" instead.\r\n");
                    return CLI_NO_ERROR;
                }
                ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL;
            }
            else if((arg[1][0] == 'l') || (arg[1][0] == 'L')) /* link-local */
            {
                ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL;

                /* check link-local address */
                if(!L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL_UNICAST(craft_addr.addr.addr))
                {
                    CLI_LIB_PrintStr("Invalid link-local address.\r\n");
                    return CLI_NO_ERROR;
                }
                /* assign zoneid */
                craft_addr.addr.zoneid = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;

            }
            else if((arg[1][0] == 'e') || (arg[1][0] == 'E')) /* eui-64 */
            {
                /* check link-local address */
                if(L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL(craft_addr.addr.addr))
                {
                    CLI_LIB_PrintStr(" Use ""ipv6 address X:X:X:X::X link-local"" instead.\r\n");
                    return CLI_NO_ERROR;
                }
                ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_EUI64;
            }
            else
            {
                CLI_LIB_PrintStr("Invalid IPv6 address format.\r\n");
            }

            craft_addr.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
            craft_addr.ipv6_addr_config_type = NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_MANUAL;
            craft_addr.ipv6_addr_type = ipv6_addr_type;
            craft_addr.row_status = VAL_netConfigStatus_2_createAndGo;

            if(craft_addr.ipv6_addr_type == NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL)
                craft_addr.addr.preflen = 64;

            /* set craft interface's ip address*/
            rc_netcfg = NETCFG_PMGR_IP_SetCraftInterfaceInetAddress(&craft_addr);

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
                case NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED:
                    CLI_LIB_PrintStr("Failed to add IPv6 address. Overlapped with existing address.\r\n");
                    return CLI_NO_ERROR;
                default:
                    /* unknown error */
                    CLI_LIB_PrintStr("Failed to add IPv6 address.\r\n");
                    return CLI_NO_ERROR;
            }

            break;
        case PRIVILEGE_CFG_INTERFACE_CRAFT_CMD_W3_NO_IPV6_ADDRESS:
            if(arg[0] == NULL)
                return CLI_ERR_INTERNAL;

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV6_ADDR_RAW,
                                                               arg[0],
                                                               (L_INET_Addr_T *)&craft_addr.addr,
                                                               sizeof(craft_addr.addr)))
            {
                CLI_LIB_PrintStr("Invalid IPv6 address format.\r\n");
                return CLI_NO_ERROR;
            }

            if(arg[1] == NULL)
            {
                /* check link-local address */
                if(L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL(craft_addr.addr.addr))
                {
                    CLI_LIB_PrintStr("Use ""no ipv6 address X:X:X:X::X link-local"" instead.\r\n");
                    return CLI_NO_ERROR;
                }
                ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL;
            }
            else if((arg[1][0] == 'l') || (arg[1][0] == 'L')) /* link-local */
            {
                ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL;

                /* check link-local address */
                if(!L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL_UNICAST(craft_addr.addr.addr))
                {
                    CLI_LIB_PrintStr("Invalid link-local address.\r\n");
                    return CLI_NO_ERROR;
                }
                /* assign zoneid */
                craft_addr.addr.zoneid = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
            }
            else if((arg[1][0] == 'e') || (arg[1][0] == 'E')) /* eui-64 */
            {
                ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_EUI64;
            }
            else
            {
                CLI_LIB_PrintStr("Invalid IPv6 address format.\r\n");
                return CLI_NO_ERROR;
            }

            craft_addr.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
            craft_addr.ipv6_addr_config_type = NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_MANUAL;
            craft_addr.ipv6_addr_type = ipv6_addr_type;

            if(craft_addr.addr.type == L_INET_ADDR_TYPE_IPV6Z)
                craft_addr.addr.preflen = 64;

            craft_addr.row_status = VAL_netConfigStatus_2_destroy;
            rc_netcfg = NETCFG_PMGR_IP_SetCraftInterfaceInetAddress(&craft_addr);

            switch(rc_netcfg)
            {
                case NETCFG_TYPE_ENTRY_NOT_EXIST:
                    CLI_LIB_PrintStr("Address not found.\r\n");
                case NETCFG_TYPE_OK:
                    return CLI_NO_ERROR;
                case NETCFG_TYPE_FAIL:
                default:
                    CLI_LIB_PrintStr("Failed to delete IPv6 address.\r\n");
                    return CLI_NO_ERROR;
            }
            break;

        default:
            CLI_LIB_PrintStr_1("(no) IPv6 address: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
    return CLI_NO_ERROR;
}

/* FUNTION NAME: CLI_API_Craft_Interface_Ipv6_Enable
 * PURPOSE:
 *      Enable or disable ipv6 process on the ipv6 interface.
 *      The common API of both commands:
 *          ipv6 enable
 *          no ipv6 enable
 *
 * INPUT:
 *      cmd_idx     -- command index
 *      arg         -- available addtional parameters array
 *      ctrl_P      -- CLI work space structure
 *
 * OUTPUT:
 *      None.
 * NOTES:
 *      None.
 */
UI32_T CLI_API_Craft_Interface_Ipv6_Enable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
   // UI32_T cli_ifindex;
    UI32_T rc_netcfg;
    NETCFG_TYPE_CraftInetAddress_T craft_addr;

    UI8_T mac_addr[6] = {0x0, 0x0, 0xe8, 0x0, 0x0, 0x1};

    /* DEBUG */
    printf("%s, %d\n", __FUNCTION__, __LINE__);

    memset(&craft_addr, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

    /* DEBUG */
    printf("%s, %d, %s\n", __FUNCTION__, __LINE__, "");


    IP_LIB_IPv6_IPv6AddressEUI64(craft_addr.addr.addr, 64, mac_addr);
    craft_addr.addr.type = L_INET_ADDR_TYPE_IPV6Z;
    craft_addr.addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    craft_addr.addr.preflen = 64;
    craft_addr.addr.zoneid = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;

    craft_addr.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
    craft_addr.ipv6_addr_config_type = NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_OTHER;
    craft_addr.ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL;

    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_CRAFT_CMD_W2_IPV6_ENABLE :


            craft_addr.row_status = VAL_netConfigStatus_2_createAndGo;

            if(NETCFG_PMGR_IP_SetCraftInterfaceInetAddress(&craft_addr) != NETCFG_TYPE_OK)
                CLI_LIB_Printf("Failed to enable IPv6 processing.\r\n");
            break;

        case PRIVILEGE_CFG_INTERFACE_CRAFT_CMD_W3_NO_IPV6_ENABLE :

            craft_addr.row_status = VAL_netConfigStatus_2_destroy;

            //rc_netcfg = NETCFG_PMGR_IP_IPv6Disable(cli_ifindex);

            rc_netcfg = NETCFG_PMGR_IP_SetCraftInterfaceInetAddress(&craft_addr);

            switch (rc_netcfg)
            {
                case NETCFG_TYPE_OK:
                    /* succeed */
                    break;
                case NETCFG_TYPE_HAS_EXPLICIT_IPV6_ADDR:
                    CLI_LIB_PrintStr("Failed to disable IPv6 processing because there is an explicit IPv6 address.\r\n");
                    break;
                default:

                    CLI_LIB_PrintStr("Failed to disable IPv6 processing.");
                    break;
            }
            break;
        default:
            CLI_LIB_Printf("(no) IPv6 enable: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
#endif
    switch (cmd_idx){
        case PRIVILEGE_CFG_INTERFACE_CRAFT_CMD_W2_IPV6_ENABLE :

            NETCFG_PMGR_IP_IPv6Enable_Craft(SYS_ADPT_CRAFT_INTERFACE_IFINDEX, TRUE);
            break;

        case PRIVILEGE_CFG_INTERFACE_CRAFT_CMD_W3_NO_IPV6_ENABLE :
            NETCFG_PMGR_IP_IPv6Enable_Craft(SYS_ADPT_CRAFT_INTERFACE_IFINDEX, FALSE);

            break;
        default:
            CLI_LIB_Printf("(no) IPv6 enable: possible CLI parsing error-invalid command index=%u.\r\n",cmd_idx);
            break;
    }
    return (UI32_T)CLI_NO_ERROR;
}
#endif /* SYS_CPNT_CRAFT_PORT */

/* command: [no] ipv6 nd raguard
 */
UI32_T  CLI_API_IPV6_Nd_RaGuard_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    UI32_T  lport   = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port=0;
    UI32_T  verify_ret=0;
    UI32_T  i = 0;
    BOOL_T  is_enable;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_IPV6_ND_RAGUARD:
        is_enable = TRUE;
        break;
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_IPV6_ND_RAGUARD:
        is_enable = FALSE;
        break;
    default:
        return CLI_NO_ERROR;
    }

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)] & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

            if (FALSE == NETCFG_PMGR_ND_RAGUARD_SetPortStatus(
                            lport, is_enable))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2(
                    "Failed to configure IPv6 RA Guard on port %lu/%lu.\r\n",
                    (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
            }
        }
    }

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T  CLI_API_IPV6_Nd_RaGuard_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    UI32_T                  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T                  lport;
    CLI_API_TrunkStatus_T   verify_ret;
    BOOL_T                  is_enable;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_IPV6_ND_RAGUARD:
        is_enable = TRUE;
        break;
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_IPV6_ND_RAGUARD:
        is_enable = FALSE;
        break;
    default:
        return CLI_NO_ERROR;
    }

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
    }
    else
    {
        if (FALSE == NETCFG_PMGR_ND_RAGUARD_SetPortStatus(
                        lport, is_enable))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1(
                "Failed to configure IPv6 RA Guard on trunk %lu.\r\n",
                (unsigned long)verify_trunk_id);
#endif
        }
    }

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
static UI32_T Show_RaGuard_Info_Title(
    char   *buff,
    UI32_T line_num)
{
    sprintf(buff, "%-9s %-8s\r\n",
                  "Interface", "RA Guard");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "%-9s %-8s\r\n",
                  "---------", "--------");
    PROCESS_MORE_FUNC(buff);
    return line_num;
}

static UI32_T Show_RaGuardInfo_One(
    char    *buff,
    UI32_T  lport,
    UI32_T  line_num)
{
    UI32_T                          unit, port, trunk_id;
    SWCTRL_Lport_Type_T             port_type;
    char                            port_str[12];
    char                            *en_str[] = {"No", "Yes"};
    char                            *en_p;
    BOOL_T                          port_status;

    port_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    if (  (port_type == SWCTRL_LPORT_TRUNK_PORT)
        ||(port_type == SWCTRL_LPORT_NORMAL_PORT)
       )
    {
        if (FALSE == NETCFG_PMGR_ND_RAGUARD_GetPortStatus(
                        lport, &port_status))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            if (port_type == SWCTRL_LPORT_TRUNK_PORT)
            {
                sprintf(buff, "Failed to display IPv6 RA Guard information on trunk %lu.\r\n",
                    (unsigned long)trunk_id);
            }
            else
            {
                sprintf(buff, "Failed to display IPv6 RA Guard information on port %lu/%lu.\r\n",
                    (unsigned long)unit, (unsigned long)port);
            }
#endif
            PROCESS_MORE_FUNC(buff);
            return line_num;
        }
    }
    else
    {
        return line_num;
    }

    if (port_type == SWCTRL_LPORT_NORMAL_PORT)
    {
        sprintf(port_str, "Eth %lu/%2lu", (unsigned long)unit, (unsigned long)port);
    }
    else
    {
        sprintf(port_str, "Trunk %lu", (unsigned long)trunk_id);
    }

    en_p    = (TRUE == port_status)?en_str[1]:en_str[0];

    sprintf(buff, "%-9s %-8s\r\n",
                    port_str, en_p);
    PROCESS_MORE_FUNC(buff);
    return line_num;

/* Sample:
 * Interface RA Guard
 * --------- --------
 *         9        8
 * Eth 1/1   Yes
 * Eth 1/2   Yes
 * Trunk 1   Yes
 */
}
#endif  /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

/* command: show ipv6 nd raguard [interface {eth|pch}]
 */
UI32_T  CLI_API_Show_IPv6_ND_RaGuard(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    UI32_T  ifindex;
    UI32_T  line_num = 1;
    UI32_T  i        = 0;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    UI32_T  verify_trunk_id = 0;
    UI32_T  max_port_num;
    UI32_T  current_max_unit = 0;
    UI32_T  j;
    CLI_API_EthStatus_T     verify_ret_e;
    CLI_API_TrunkStatus_T   verify_ret_t;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

#if (SYS_CPNT_STACKING == TRUE)
    current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
    current_max_unit = 1;
#endif

    if (arg[0] == NULL)
    {
        line_num = Show_RaGuard_Info_Title(buff, line_num);
        for (j=0; STKTPLG_POM_GetNextUnit(&j); )
        {
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
            for (i = 1; i <= max_port_num ; i++)
            {
                verify_unit = j;
                verify_port = i;

                if ((verify_ret_e = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
                {
                    continue;
                }

                if ((line_num = Show_RaGuardInfo_One(buff, ifindex, line_num)) == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
                if (line_num == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
            }
        }/*end of unit loop*/

        /*trunk*/
        while (TRK_PMGR_GetNextTrunkId(&verify_trunk_id))
        {
            if ((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
            {
                continue;
            }
            if ((line_num = Show_RaGuardInfo_One(buff, ifindex, line_num)) == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
            if (line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }
    }
    else
    {
        switch(arg[1][0])
        {
        case 'e':
        case 'E':
            verify_unit = atoi(arg[2]);
            verify_port = atoi(strchr(arg[2], '/') + 1);

            if ((verify_ret_e = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }
            break;

        case 'p':
        case 'P':
            verify_trunk_id = atoi(arg[2]);
            if ((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret_t, verify_trunk_id);
                return CLI_NO_ERROR;
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
        }

        line_num = Show_RaGuard_Info_Title(buff, line_num);
        Show_RaGuardInfo_One(buff, ifindex, line_num);
    }
#endif  /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

    return CLI_NO_ERROR;
}

#endif // #if (SYS_CPNT_IPV6 == TRUE)

