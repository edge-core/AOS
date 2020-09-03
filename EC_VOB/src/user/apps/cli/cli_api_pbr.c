/* MODULE NAME: cli_api_pbr.c
 * PURPOSE:
 *   Definitions of CLI APIs for PBR.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   2015/07/10     KH Shi, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

 /* INCLUDE FILE DECLARATIONS
  */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "cli_api.h"
#include "cli_lib.h"
#include "l_inet.h"
#include "vlan_lib.h"
#include "cli_api_pbr.h"
#include "netcfg_type.h"
#include "netcfg_pom_pbr.h"
#include "netcfg_pmgr_pbr.h"

#if (SYS_CPNT_PBR == TRUE)
static char *pbr_route_map_type_str[]   = {"permit", "deny", "any"};
#endif

/* EXPORTED SUBPROGRAM BODIES
 */
UI32_T CLI_API_PBR_IpPolicyRouteMap(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PBR == TRUE)
    char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1] = {0};
    UI32_T vid = 0;
    UI32_T ret = 0;
    
    VLAN_IFINDEX_CONVERTTO_VID(ctrl_P->CMenu.vlan_ifindex, vid);
    
    if (arg[0])
    {
        strncpy(rmap_name, arg[0], SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IP_POLICY_ROUTEMAP:
            if (NETCFG_TYPE_OK != NETCFG_PMGR_PBR_BindRouteMap(vid, rmap_name))
            {
                CLI_LIB_Printf("Failed to bind route-map to VLAN.\r\n");
            }
            return CLI_NO_ERROR;
            
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IP_POLICY_ROUTEMAP:
            if(NETCFG_TYPE_OK != NETCFG_PMGR_PBR_UnbindRouteMap(vid))
            {
                CLI_LIB_Printf("Failed to unbind route-map from VLAN.\r\n");
            }
            return CLI_NO_ERROR;

        default:
            break;
    }    
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_PBR_ShowIpPolicy(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PBR == TRUE)
    NETCFG_OM_PBR_BindingEntry_T  pbr_binding;
    UI32_T  line_num =0;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char    ip_buf[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
    UI32_T  previous_vid = 0;

    memset(&pbr_binding, 0, sizeof(NETCFG_OM_PBR_BindingEntry_T));

    while (NETCFG_POM_PBR_GetNextBindingEntry(&pbr_binding))
    {
        if (pbr_binding.seq_num == 0) 
        {
            sprintf(buff, "Vlan: %lu, Route map: %s, Details is below:\r\n", (unsigned long)pbr_binding.vid, pbr_binding.rmap_name);
            PROCESS_MORE(buff);
            continue;
        }

        sprintf(buff, "  type: %s\r\n", pbr_route_map_type_str[pbr_binding.rmap_type]);
        PROCESS_MORE(buff);

        sprintf(buff, "  sequence: %lu\r\n", (unsigned long)pbr_binding.seq_num);
        PROCESS_MORE(buff);

        if (strlen(pbr_binding.acl_name) > 0)
        {
            sprintf(buff, "  match ip address: %s\r\n", pbr_binding.acl_name);
            PROCESS_MORE(buff);
        }

        if (pbr_binding.option & NETCFG_OM_PBR_BINDING_OPT_NEXTHOP)
        {
            L_INET_InaddrToString((L_INET_Addr_T *)(&pbr_binding.nexthop), ip_buf, sizeof(ip_buf));
            sprintf(buff, "  set ip nexthop: %s\r\n", ip_buf);
            PROCESS_MORE(buff);
        }

        if (pbr_binding.option & NETCFG_OM_PBR_BINDING_OPT_DSCP)
        {
            sprintf(buff, "  set ip dscp: %lu\r\n", (unsigned long)pbr_binding.dscp);
            PROCESS_MORE(buff);
        }

        if (pbr_binding.in_chip)
        {
            PROCESS_MORE("  activated: true\r\n");
        }
        else
        {
            PROCESS_MORE("  activated: false\r\n");
        }

        PROCESS_MORE("\r\n");
    }
#endif
    return CLI_NO_ERROR;
}
 
