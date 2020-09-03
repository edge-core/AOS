 /* MODULE NAME: cli_api_vxlan.c
 * PURPOSE:
 *   Provides the definitions for VXLAN APIs.
 * NOTES:
 *   None
 *
 * HISTORY:
 *   04/21/2015 -- Kelly Chen, Create
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

#if (SYS_CPNT_VXLAN == TRUE)
#include "l_inet.h"
#include "cli_api_vxlan.h"
#include "vxlan_pmgr.h"
#include "vxlan_type.h"
#include "vxlan_pom.h"
#include "swctrl.h"
#include "ip_lib.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

 /* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DEFINITIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME - CLI_API_VXLAN_UdpDstPort
 * PURPOSE : Set UDP port number.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_UdpDstPort(UI16_T cmd_idx, char *arg[],
                                CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VXLAN == TRUE)
    UI32_T ret;
    UI16_T dst_port = 0;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_VXLAN_UDPDSTPORT:
            if(NULL == arg[0])
            {
                return CLI_ERR_INTERNAL;
            }

            dst_port = atol(arg[0]);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_VXLAN_UDPDSTPORT:
            dst_port = VXLAN_TYPE_DFLT_UDP_DST_PORT;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    ret = VXLAN_PMGR_SetUdpDstPort(dst_port);
    if (VXLAN_TYPE_ENTRY_EXISTED == ret)
    {
        CLI_LIB_PrintStr("Failed to set VXLAN UDP port because VTEP is created.\r\n");
    }
    else if (VXLAN_PMGR_SetUdpDstPort(dst_port) != VXLAN_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr("Failed to set VXLAN UDP port.\r\n");
    }
#endif /* End #if (SYS_CPNT_VXLAN == TRUE) */
    return CLI_NO_ERROR;
}

/* FUNCTION NAME - CLI_API_VXLAN_Vni
 * PURPOSE : Configure VNI
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_Vni(UI16_T cmd_idx, char *arg[],
                                CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VXLAN == TRUE)
    L_INET_AddrIp_T ip_p;
    BOOL_T set_m =FALSE;
    UI32_T vni = 0,lport = 0;
    UI32_T ret;
    UI16_T vid = 0;

    memset(&ip_p, 0, sizeof(L_INET_AddrIp_T));
    if(NULL == arg[0])
    {
        return CLI_ERR_INTERNAL;
    }

    vni = atol(arg[0]);

    if (NULL == arg[1])
    {
        switch(cmd_idx)
        {
            case PRIVILEGE_CFG_GLOBAL_CMD_W2_VXLAN_VNI:
                ret = VXLAN_PMGR_AddVpn(vni);
                if (ret == VXLAN_TYPE_ENTRY_EXISTED)
                {
                    CLI_LIB_PrintStr_1("VNI %lu already exist.\r\n", (unsigned long)vni);
                }
                else if (ret != VXLAN_TYPE_RETURN_OK)
                {
                    CLI_LIB_PrintStr_1("Fail to add VNI %lu.\r\n", (unsigned long)vni);
                }
                break;
            case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_VXLAN_VNI:
                ret = VXLAN_PMGR_DeleteVpn(vni);
                if (ret == VXLAN_TYPE_ENTRY_NOT_EXISTED)
                {
                    CLI_LIB_PrintStr_1("VNI %lu not exist.\r\n", (unsigned long)vni);
                }
                else if (ret == VXLAN_TYPE_ENTRY_EXISTED)
                {
                    CLI_LIB_PrintStr_1("VNI %lu had associated with flood/multicast group.\r\n", (unsigned long)vni);
                }
                else if (ret != VXLAN_TYPE_RETURN_OK)
                {
                    CLI_LIB_PrintStr_1("Fail to delete VNI %lu.\r\n", (unsigned long)vni);
                }
                break;
            default:
                return CLI_ERR_INTERNAL;
        }

        return CLI_NO_ERROR;
    }

    if (arg[1][0] == 'f' || arg[1][0] == 'F')
    {
        if (arg[2][0] == 'm' || arg[2][0] == 'M')
        {
            set_m = TRUE;
        }
        else if (arg[2][0] == 'r' || arg[2][0] == 'R')
        {
            set_m = FALSE;
        }
        else
        {
            return CLI_ERR_INTERNAL;
        }

        switch(cmd_idx)
        {
            case PRIVILEGE_CFG_GLOBAL_CMD_W2_VXLAN_VNI:

                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                                   arg[3],
                                                                   (L_INET_Addr_T *) &ip_p,
                                                                   sizeof(ip_p)))
                {
                    CLI_LIB_PrintStr("Invalid address format.\r\n");
                    return CLI_NO_ERROR;
                }

                if (set_m)
                {
                    vid = atol(arg[5]);
                    if (arg[6][0] == 'e' || arg[6][0] == 'E')
                    {
                        UI32_T verify_unit = (UI32_T)atoi((char*)arg[7]);
                        UI32_T verify_port = atoi(strchr((char*)arg[7], '/')+1 );
                        CLI_API_EthStatus_T verify_ret;

                        if ((verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                        {
                            display_ethernet_msg(verify_ret, verify_unit, verify_port);
                            return CLI_NO_ERROR;
                        }
                    }

                    ret = VXLAN_PMGR_AddFloodMulticast(vni, &ip_p, vid, lport);
                    if (ret == VXLAN_TYPE_IP_INVALID)
                    {
                        CLI_LIB_PrintStr_1("Invalid IP address %s.\r\n", arg[3]);
                    }
                    else if (ret == VXLAN_TYPE_VNI_NOT_MATCH)
                    {
                        CLI_LIB_PrintStr_1("Not find VNI %lu.\r\n", (unsigned long)vni);
                    }
                    else if (ret == VXLAN_TYPE_SRC_IP_NOT_FIND)
                    {
                        CLI_LIB_PrintStr_1("Warning: No local IP address of VLAN %u.\r\n", vid);
                    }
                    else if (ret == VXLAN_TYPE_SRC_IF_NOT_FIND)
                    {
                        CLI_LIB_PrintStr("Warning: No configure source interface\r\n");
                    }
                    else if (ret == VXLAN_TYPE_SRC_IF_IP_NOT_FIND)
                    {
                        CLI_LIB_PrintStr("Warning: No configure primary IP on source interface\r\n");
                    }
                    else if (ret == VXLAN_TYPE_ENTRY_EXISTED)
                    {
                        CLI_LIB_PrintStr_1("VNI %lu had associated with multicast group.\r\n", (unsigned long)vni);
                    }
                    else if (ret == VXLAN_TYPE_TABLE_FULL)
                    {
                        CLI_LIB_PrintStr("VTEP table is full.\r\n");
                    }
                    else if (ret == VXLAN_TYPE_ACCESS_PORT_NOT_FIND)
                    {
                        CLI_LIB_PrintStr("Warning: No Access Port.\r\n");
                    }
                    else if (ret != VXLAN_TYPE_RETURN_OK)
                    {
                        CLI_LIB_PrintStr_1("Failed to add multicast group IP address %s.\r\n", arg[3]);
                    }
                }
                else
                {
                    ret = VXLAN_PMGR_AddFloodRVtep(vni, &ip_p);
                    if (ret == VXLAN_TYPE_IP_INVALID)
                    {
                        CLI_LIB_PrintStr_1("Invalid IP address %s.\r\n", arg[3]);
                    }
                    else if (ret == VXLAN_TYPE_ROUTE_NOT_FIND)
                    {
                        CLI_LIB_PrintStr_1("Warning: No route found for IP address %s.\r\n", arg[3]);
                    }
                    else if (ret == VXLAN_TYPE_HOST_NOT_FIND)
                    {
                        CLI_LIB_PrintStr_1("Warning: No nexthop information found for IP address %s.\r\n", arg[3]);
                    }
                    else if (ret == VXLAN_TYPE_SRC_IF_NOT_FIND)
                    {
                        CLI_LIB_PrintStr("Warning: No configure source interface\r\n");
                    }
                    else if (ret == VXLAN_TYPE_SRC_IF_IP_NOT_FIND)
                    {
                        CLI_LIB_PrintStr("Warning: No configure primary IP on source interface\r\n");
                    }
                    else if (ret == VXLAN_TYPE_TABLE_FULL)
                    {
                        CLI_LIB_PrintStr("VTEP table is full.\r\n");
                    }
                    else if (ret == VXLAN_TYPE_ACCESS_PORT_NOT_FIND)
                    {
                        CLI_LIB_PrintStr("Warning: No Access Port.\r\n");
                    }
                    else if (ret != VXLAN_TYPE_RETURN_OK)
                    {
                        CLI_LIB_PrintStr_1("Failed to add flooding IP address %s.\r\n", arg[3]);
                    }
                }
                break;

            case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_VXLAN_VNI:

                if (set_m)
                {
                    if (VXLAN_PMGR_DelFloodMulticast(vni) != VXLAN_TYPE_RETURN_OK)
                    {
                        CLI_LIB_PrintStr("Failed to delete multicast group IP address.\r\n");
                    }
                }
                else
                {
                    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                                       arg[3],
                                                                       (L_INET_Addr_T *) &ip_p,
                                                                       sizeof(ip_p)))
                    {
                        CLI_LIB_PrintStr("Invalid address format.\r\n");
                        return CLI_NO_ERROR;
                    }

                    if (VXLAN_PMGR_DelFloodRVtep(vni, &ip_p) != VXLAN_TYPE_RETURN_OK)
                    {
                        CLI_LIB_PrintStr_1("Failed to delete flooding IP address %s.\r\n", arg[3]);
                    }
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }
    else if (arg[1][0] == 'a' || arg[1][0] == 'A') /* access-port */
    {
        if (arg[3][0] == 'e' || arg[3][0] == 'E') /* interface ethernet */
        {
            UI32_T verify_unit = (UI32_T)atoi((char*)arg[4]);
            UI32_T verify_port = atoi(strchr((char*)arg[4], '/')+1 );
            CLI_API_EthStatus_T verify_ret;

            if ((verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }

            vid = 0;
            if (arg[5] != NULL)
                vid = atol(arg[6]);

            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_GLOBAL_CMD_W2_VXLAN_VNI:
                    if (VXLAN_PMGR_SetPortVlanVniMap(lport, vid, vni, TRUE) != VXLAN_TYPE_RETURN_OK)
                        CLI_LIB_PrintStr("Failed to create access port.\r\n");
                    break;
                case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_VXLAN_VNI:
                    VXLAN_PMGR_SetPortVlanVniMap(lport, vid, vni, FALSE);
                    break;
                default:
                    return CLI_ERR_INTERNAL;
            }
        }
    }
    else
    {
        return CLI_ERR_INTERNAL;
    }


#endif /* End #if (SYS_CPNT_VXLAN == TRUE) */
    return CLI_NO_ERROR;

}

/* FUNCTION NAME - CLI_API_VXLAN_FloodRVTEP
 * PURPOSE : looding to which remote VTEP, when received packet lookup bridge table fail.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_FloodRVTEP(UI16_T cmd_idx, char *arg[],
                                CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VXLAN == TRUE)
    L_INET_AddrIp_T ip_p;
    UI32_T vni = 0;


    memset(&ip_p, 0, sizeof(L_INET_AddrIp_T));
    if(NULL == arg[0])
    {
        return CLI_ERR_INTERNAL;
    }
    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                       arg[0],
                                                       (L_INET_Addr_T *) &ip_p,
                                                       sizeof(ip_p)))
    {
        CLI_LIB_PrintStr("Invalid address format.\r\n");
        return CLI_NO_ERROR;
    }
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_VXLAN_FLOOD_RVTEP:
            if (VXLAN_PMGR_AddFloodRVtep(vni, &ip_p) != VXLAN_TYPE_RETURN_OK)
            {
                CLI_LIB_PrintStr_1("Failed to add flooding IP address %s.\r\n", arg[0]);
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_VXLAN_FLOOD_RVTEP:
            if (VXLAN_PMGR_DelFloodRVtep(vni, &ip_p) != VXLAN_TYPE_RETURN_OK)
            {
                CLI_LIB_PrintStr_1("Failed to delete flooding IP address %s.\r\n", arg[0]);
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

#endif /* End #if (SYS_CPNT_VXLAN == TRUE) */
    return CLI_NO_ERROR;

}

/* FUNCTION NAME - CLI_API_VXLAN_FloodMulticast
 * PURPOSE : looding to which remote VTEP, when received packet lookup bridge table fail.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_FloodMulticast(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VXLAN == TRUE)
    L_INET_AddrIp_T m_ip_p;
    UI32_T vni = 0,lport = 0;
    UI16_T vid = 0;

    memset(&m_ip_p, 0, sizeof(L_INET_AddrIp_T));

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_VXLAN_FLOOD_MULTICAST:
            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                               arg[0],
                                                               (L_INET_Addr_T *) &m_ip_p,
                                                               sizeof(m_ip_p)))
            {
                CLI_LIB_PrintStr("Invalid address format.\r\n");
                return CLI_NO_ERROR;
            }
            vid = atol(arg[2]);
            if (arg[3][0] == 'e' || arg[3][0] == 'E')
            {
                UI32_T verify_unit = (UI32_T)atoi((char*)arg[4]);
                UI32_T verify_port = atoi(strchr((char*)arg[4], '/')+1 );
                CLI_API_EthStatus_T verify_ret;

                if ((verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                    display_ethernet_msg(verify_ret, verify_unit, verify_port);
                    return CLI_NO_ERROR;
                }
            }

            /*  vni = 0 , add multicast group to all vni.
             */
            if (VXLAN_PMGR_AddFloodMulticast(vni, &m_ip_p, vid, lport) != VXLAN_TYPE_RETURN_OK)
            {
                CLI_LIB_PrintStr("Failed to add multicast group IP address.\r\n");
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_VXLAN_FLOOD_MULTICAST:
            /*  vni = 0 , delete multicast group from all vni.
             */
            if (VXLAN_PMGR_DelFloodMulticast(vni) != VXLAN_TYPE_RETURN_OK)
            {
                CLI_LIB_PrintStr("Failed to delete multicast group IP address.\r\n");
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

#endif /* End #if (SYS_CPNT_VXLAN == TRUE) */
    return CLI_NO_ERROR;
}

/* FUNCTION NAME - CLI_API_VXLAN_VlanVniMap
 * PURPOSE : Configure VLAN and VNI mapping relationship.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_VlanVniMap(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0    
#if (SYS_CPNT_VXLAN == TRUE)
    BOOL_T is_add = TRUE;
    UI32_T vni = 0;
    UI32_T ret;
    UI16_T vid = 0;

    if(NULL == arg[0])
    {
        return CLI_ERR_INTERNAL;
    }

    vid = atol(arg[0]);
    vni = atol(arg[2]);
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_VXLAN_VLAN:
            is_add = TRUE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_VXLAN_VLAN:
            is_add = FALSE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    ret = VXLAN_PMGR_SetVlanVniMap(vid, vni, is_add);

    if (ret == VXLAN_TYPE_VNI_NOT_MATCH)
    {
        CLI_LIB_PrintStr_1("Specify a wrong VNI %lu.\r\n", (unsigned long)vni);
    }
    if (ret == VXLAN_TYPE_ENTRY_EXISTED)
    {
        CLI_LIB_PrintStr("Have VTEP port.\r\n");
    }
    else if (ret != VXLAN_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr_2("Failed to associate VLAN %u with VNI %lu.\r\n", vid, (unsigned long)vni);
    }
#endif /* End #if (SYS_CPNT_VXLAN == TRUE) */
#endif
    return CLI_NO_ERROR;
}


/* FUNCTION NAME - CLI_API_VXLAN_SrcInterface
 * PURPOSE : Specify source interface of local VTEP.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_SrcInterface(UI16_T cmd_idx, char *arg[],
                           CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VXLAN == TRUE)
    UI32_T vid = 0, loopback_id = 0;
    UI32_T ifindex = 0;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_VXLAN_SOURCEINTERFACE:
            if (NULL == arg[0] || NULL == arg[1])
            {
                return CLI_ERR_INTERNAL;
            }
            if (arg[0][0] == 'v' || arg[0][0] == 'V')
            {
                vid = atol(arg[1]);
                if (vid != 0)
                    VLAN_OM_ConvertToIfindex(vid, &ifindex);
            }
            else if (arg[0][0] == 'l' || arg[0][0] == 'L')
            {
                loopback_id = atol(arg[1]);
                IP_LIB_ConvertLoopbackIdToIfindex(loopback_id, &ifindex);
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_VXLAN_SOURCEINTERFACE:
            ifindex = 0;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (VXLAN_PMGR_SetSrcIf(ifindex) != VXLAN_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr("Failed to set source interface VLAN.\r\n");
    }

#endif /* End #if (SYS_CPNT_VXLAN == TRUE) */
    return CLI_NO_ERROR;
}

/* FUNCTION NAME - CLI_API_VXLAN_ShowUdpDstPort
 * PURPOSE : Show VXLAN UDP port number.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_ShowUdpDstPort(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VXLAN == TRUE)
    UI16_T dst_port = 0;

    if (VXLAN_POM_GetUdpDstPort(&dst_port) != VXLAN_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr("Failed to get VXLAN UDP port.\r\n");
        return CLI_NO_ERROR;
    }

    CLI_LIB_Printf("  VXLAN UDP Destination Port: %d\r\n", dst_port);
#endif /* End #if (SYS_CPNT_VXLAN == TRUE) */
    return CLI_NO_ERROR;
}

/* FUNCTION NAME - CLI_API_VXLAN_ShowVtep
 * PURPOSE : Show VXLAN tunnel created situation.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_ShowVtep(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_OM_RVtep_T rvtep_entry;
    UI32_T vni = 0;
    UI32_T  type;
    UI32_T  unit;
    UI32_T  port;
    UI32_T  trunk_id;
    UI32_T  line_num = 0;
    UI32_T  i = 0;
    char    s_ip_ar[20] = {0};
    char    r_ip_ar[20] = {0};
    char    n_ip_ar[20] = {0};
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  VNI      SIP             R-VTEP          Nexthop         Port\r\n");
    PROCESS_MORE(buff);
    PROCESS_MORE("  -------- --------------- --------------- --------------- --------\r\n");
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    while (VXLAN_POM_GetNextFloodRVtep(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        memset(s_ip_ar, 0, sizeof(s_ip_ar));
        memset(r_ip_ar, 0, sizeof(r_ip_ar));
        SYSFUN_Snprintf(s_ip_ar, sizeof(s_ip_ar), "%d.%d.%d.%d",
            rvtep_entry.s_ip.addr[0], rvtep_entry.s_ip.addr[1],
            rvtep_entry.s_ip.addr[2], rvtep_entry.s_ip.addr[3]);
        SYSFUN_Snprintf(r_ip_ar, sizeof(r_ip_ar), "%d.%d.%d.%d",
            rvtep_entry.ip.addr[0], rvtep_entry.ip.addr[1],
            rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
        for (i=0; i<rvtep_entry.nexthop_cnt; i++)
        {
            SYSFUN_Snprintf(n_ip_ar, sizeof(n_ip_ar), "%d.%d.%d.%d",
                rvtep_entry.nexthops_ip_ar[i].addr[0], rvtep_entry.nexthops_ip_ar[i].addr[1],
                rvtep_entry.nexthops_ip_ar[i].addr[2], rvtep_entry.nexthops_ip_ar[i].addr[3]);
            type = SWCTRL_POM_LogicalPortToUserPort(rvtep_entry.nexthops_lport_ar[i], &unit, &port, &trunk_id);
            if (i == 0)
            {
                if(type == SWCTRL_LPORT_NORMAL_PORT)
                {
                    SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %8lu %-15s %-15s %-15s Eth%lu/%lu\r\n",
                        (unsigned long)rvtep_entry.vni, s_ip_ar, r_ip_ar, n_ip_ar, (unsigned long)unit, (unsigned long)port);
                    PROCESS_MORE(buff);
                }
                else if (type == SWCTRL_LPORT_TRUNK_PORT)
                {
                    SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %8lu %-15s %-15s %-15s Trunk %lu\r\n",
                        (unsigned long)rvtep_entry.vni, s_ip_ar, r_ip_ar, n_ip_ar, (unsigned long)trunk_id);
                    PROCESS_MORE(buff);
                }
                else if (type == SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %8lu %-15s %-15s %-15s\r\n",
                        (unsigned long)rvtep_entry.vni, s_ip_ar, r_ip_ar, n_ip_ar);
                    PROCESS_MORE(buff);
                }
            }
            else
            {
                if(type == SWCTRL_LPORT_NORMAL_PORT)
                {
                    SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %40s %-15s Eth%lu/%lu\r\n",
                        "", n_ip_ar, (unsigned long)unit, (unsigned long)port);
                    PROCESS_MORE(buff);
                }
                else if (type == SWCTRL_LPORT_TRUNK_PORT)
                {
                    SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %40s %-15s Trunk %lu\r\n",
                        "", n_ip_ar, (unsigned long)trunk_id);
                    PROCESS_MORE(buff);
                }
                else if (type == SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %40s %-15s\r\n",
                        "", n_ip_ar);
                    PROCESS_MORE(buff);
                }
            }
        }
    }
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    while (VXLAN_POM_GetNextFloodMulticast(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        type = SWCTRL_POM_LogicalPortToUserPort(rvtep_entry.lport, &unit, &port, &trunk_id);

        memset(s_ip_ar, 0, sizeof(s_ip_ar));
        memset(r_ip_ar, 0, sizeof(r_ip_ar));
        SYSFUN_Snprintf(s_ip_ar, sizeof(s_ip_ar), "%d.%d.%d.%d",
            rvtep_entry.s_ip.addr[0], rvtep_entry.s_ip.addr[1],
            rvtep_entry.s_ip.addr[2], rvtep_entry.s_ip.addr[3]);
        SYSFUN_Snprintf(r_ip_ar, sizeof(r_ip_ar), "%d.%d.%d.%d",
            rvtep_entry.ip.addr[0], rvtep_entry.ip.addr[1],
            rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
        if(type == SWCTRL_LPORT_NORMAL_PORT)
        {
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %8lu %-15s %-15s %-15s Eth%lu/%lu\r\n",
                (unsigned long)rvtep_entry.vni, s_ip_ar, r_ip_ar, "", (unsigned long)unit, (unsigned long)port);
            PROCESS_MORE(buff);
        }
        else if (type == SWCTRL_LPORT_UNKNOWN_PORT)
        {
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %8lu %-15s %-15s\r\n",
                (unsigned long)rvtep_entry.vni, s_ip_ar, r_ip_ar);
            PROCESS_MORE(buff);
        }
    }
#endif /* End #if (SYS_CPNT_VXLAN == TRUE) */
    return CLI_NO_ERROR;
}

/* FUNCTION NAME - CLI_API_VXLAN_ShowFlood
 * PURPOSE : Show VXLAN flooding configuration.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_ShowFlood(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VXLAN == TRUE)
    UI32_T vni = 0;
    VXLAN_OM_RVtep_T rvtep_entry;
    UI32_T  line_num = 0;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    if(NULL == arg[0])
    {
        SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  VNI       Remote VTEP IP address\r\n");
        PROCESS_MORE(buff);
        PROCESS_MORE("  --------  ----------------------\r\n");
        memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
        while (VXLAN_POM_GetNextFloodRVtep(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
        {
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %8lu  %d.%d.%d.%d\r\n",
                (unsigned long)rvtep_entry.vni, rvtep_entry.ip.addr[0], rvtep_entry.ip.addr[1],
                rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
            PROCESS_MORE(buff);
        }
        memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
        while (VXLAN_POM_GetNextFloodMulticast(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
        {
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %8lu  %d.%d.%d.%d\r\n",
                (unsigned long)rvtep_entry.vni, rvtep_entry.ip.addr[0], rvtep_entry.ip.addr[1],
                rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
            PROCESS_MORE(buff);
        }
    }
    else
    {
        if (NULL == arg[1])
        {
            return CLI_ERR_INTERNAL;
        }
        SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  VNI       Remote VTEP IP address\r\n");
        PROCESS_MORE(buff);
        PROCESS_MORE("  --------  ----------------------\r\n");
        memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
        rvtep_entry.vni = atol(arg[1]);
        while (VXLAN_POM_GetNextFloodRVtepByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
        {
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %8lu  %d.%d.%d.%d\r\n",
                (unsigned long)rvtep_entry.vni, rvtep_entry.ip.addr[0], rvtep_entry.ip.addr[1],
                rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
            PROCESS_MORE(buff);
        }
        memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
        rvtep_entry.vni = atol(arg[1]);
        if (VXLAN_POM_GetFloodMulticastByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
        {
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %8lu  %d.%d.%d.%d\r\n",
                (unsigned long)rvtep_entry.vni, rvtep_entry.ip.addr[0], rvtep_entry.ip.addr[1],
                rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
            PROCESS_MORE(buff);
        }
    }
#endif /* End #if (SYS_CPNT_VXLAN == TRUE) */
    return CLI_NO_ERROR;
}

/* FUNCTION NAME - CLI_API_VXLAN_ShowAccessPort
 * PURPOSE : Show VXLAN access port configuration.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_ShowAccessPort(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VXLAN == TRUE)
    UI32_T  lport = 0, vxlan_port = 0;
    UI32_T  line_num = 0;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI16_T  vid = 0;

    SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, " VLAN LPort VxLAN Port\r\n");
    PROCESS_MORE(buff);
    PROCESS_MORE(" ---- ----- ----------\r\n");

    while (TRUE == VXLAN_POM_GetNextAccessVxlanPort(&vid, &lport, &vxlan_port))
    {
        SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, " %4lu %5lu 0x%8x\r\n", 
            (unsigned long)vid, (unsigned long)lport, vxlan_port);
        PROCESS_MORE(buff);
    }

    {
    UI32_T vni = 0;
    VXLAN_OM_VNI_T vni_entry;
    SWCTRL_Lport_Type_T lport_type;
    UI32_T unit, port, trunk_id;
    char port_ar[20];

    if (NULL != arg[0])
        vni = atol(arg[0]);

    SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, " VNI      Interface VLAN\r\n");
    PROCESS_MORE(buff);
    PROCESS_MORE(" -------- --------- ----\r\n");

    memset(&vni_entry, 0, sizeof(vni_entry));
    while (VXLAN_TYPE_RETURN_OK == VXLAN_POM_GetNextVniEntry(&vni_entry))
    {
        if (vni != 0 && vni != vni_entry.vni)
            continue;

        lport = 0;
        vid = 0;
        while (VXLAN_TYPE_RETURN_OK == VXLAN_POM_GetNextPortVlanVniMapByVni(vni_entry.vni, &lport, &vid))
        {
            lport_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
            if(lport_type == SWCTRL_LPORT_NORMAL_PORT)
            {
                snprintf(port_ar, sizeof(port_ar), "Eth %lu/%lu", (unsigned long)unit, (unsigned long)port);
            }
            else if (lport_type == SWCTRL_LPORT_TRUNK_PORT)
            {
                snprintf(port_ar, sizeof(port_ar), "Trunk %lu", (unsigned long)trunk_id);
            }
            else
            {
                continue;
            }

            if (vid == 0)
            {
                SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, " 0x%06X %-9s\n",
                    vni_entry.vni, port_ar);
            }
            else
            {
                SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, " 0x%06X %-9s %4u\n",
                    vni_entry.vni, port_ar, vid);
            }
            PROCESS_MORE(buff);
        }
    }
    }
#endif /* End #if (SYS_CPNT_VXLAN == TRUE) */
    return CLI_NO_ERROR;
}


/* FUNCTION NAME - CLI_API_VXLAN_ShowVlanVniMap
 * PURPOSE : Show VLAN and VNI mapping relationship.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_ShowVlanVniMap(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VXLAN == TRUE)
    UI16_T vid = 0;
    UI32_T vni = 0;
    BOOL_T first = TRUE;
    UI32_T  line_num = 0;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    VXLAN_OM_VNI_T vni_entry;

    memset(&vni_entry, 0, sizeof(vni_entry));

    while (VXLAN_POM_GetNextVniEntry(&vni_entry) == VXLAN_TYPE_RETURN_OK)
    {
        if (first == TRUE)
        {
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, " VNI      VFI      Access Port Count\r\n");
            PROCESS_MORE(buff);
            PROCESS_MORE(" -------- -------- -----------------\r\n");
            first = FALSE;
        }
        SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, " 0x%06X 0x%06X %17lu\r\n",
            vni_entry.vni, vni_entry.vfi, (unsigned long)vni_entry.nbr_of_acc_port);
        PROCESS_MORE(buff);
    }
#if 0
    if(NULL == arg[0])
    {
        while (VXLAN_POM_GetNextVlanVniMapEntry(&vid, &vni) == VXLAN_TYPE_RETURN_OK)
        {
            if (first == TRUE)
            {
                SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  VLAN  VNI\r\n");
                PROCESS_MORE(buff);
                PROCESS_MORE("  ----  --------\r\n");
                first = FALSE;
            }
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %4u  %8lu\r\n", vid, (unsigned long)vni);
            PROCESS_MORE(buff);
        }
    }
    else
    {
        vid = atol(arg[0]);
        if (VXLAN_POM_GetVlanVniMapEntry(vid, &vni) == VXLAN_TYPE_RETURN_OK)
        {
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  VLAN  VNI\r\n");
            PROCESS_MORE(buff);
            PROCESS_MORE("  ----  --------\r\n");
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "  %4u  %8lu\r\n", vid, (unsigned long)vni);
            PROCESS_MORE(buff);
        }
    }
#endif
#endif /* End #if (SYS_CPNT_VXLAN == TRUE) */
    return CLI_NO_ERROR;
}

/* FUNCTION NAME - CLI_API_VXLAN_ShowSrcIf
 * PURPOSE : Show source interface of local VTEP.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_ShowSrcIf(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VXLAN == TRUE)
    UI32_T vid, ifindex = 0;

    if (VXLAN_POM_GetSrcIf(&ifindex) != VXLAN_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr("Failed to get VXLAN source interface.\r\n");
        return CLI_NO_ERROR;
    }

    if (IS_VLAN_IFINDEX_VAILD(ifindex))
    {
        VLAN_OM_ConvertFromIfindex(ifindex, &vid);
        CLI_LIB_Printf("  VXLAN Source Interface VLAN: %d\r\n", vid);
    }
#if (SYS_CPNT_LOOPBACK_IF_VISIBLE == TRUE)
    else if (ifindex >= SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER &&
             ifindex < SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER + SYS_ADPT_MAX_NBR_OF_LOOPBACK_IF)
    {
        UI32_T lo_id;
        IP_LIB_ConvertLoopbackIfindexToId(ifindex, &lo_id);
        CLI_LIB_Printf("  VXLAN Source Interface Loopback: %d\r\n", lo_id);
    }
#endif
#endif /* End #if (SYS_CPNT_VXLAN == TRUE) */
    return CLI_NO_ERROR;
}

/* LOCAL SUBPROGRAM BODIES
 */
