#include <stdio.h>
#include <ctype.h>
#include "l_cvrt.h"
#include "l_stdlib.h"
#include "cli_api.h"
#include "ip_lib.h"
#include "netcfg_pmgr_ip.h"
#include "netcfg_pom_ip.h"

#include "netcfg_type.h"

/*change mode*/
UI32_T CLI_API_Interface_Loopback(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LOOPBACK_IF_VISIBLE == TRUE)
    UI8_T  previous_port_id_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST] = {0};
    UI32_T previous_vlan  = 0;
    UI32_T previous_trunk = 0;
    UI32_T previous_loopback = 0;
    UI32_T ret;

    /*store previous interface information, and init working space*/
    memcpy(previous_port_id_list, ctrl_P->CMenu.port_id_list, sizeof(ctrl_P->CMenu.port_id_list));
    previous_vlan = ctrl_P->CMenu.vlan_ifindex;
    previous_trunk = ctrl_P->CMenu.pchannel_id;
    previous_loopback = ctrl_P->CMenu.loopback_id;
    memset(ctrl_P->CMenu.port_id_list, 0, sizeof(ctrl_P->CMenu.port_id_list));
    ctrl_P->CMenu.vlan_ifindex = 0;
    ctrl_P->CMenu.pchannel_id = 0;
    ctrl_P->CMenu.loopback_id = 0;

    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;

    if(0 != atoi((char*)arg[0]))
    {
        /*restore previous setting*/
        memcpy(ctrl_P->CMenu.port_id_list, previous_port_id_list, sizeof(ctrl_P->CMenu.port_id_list));
        ctrl_P->CMenu.vlan_ifindex = previous_vlan;
        ctrl_P->CMenu.pchannel_id = previous_trunk;
        ctrl_P->CMenu.loopback_id = previous_loopback;
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("No such Loopback interface.\r\n");
#endif
        return CLI_NO_ERROR;
    }

    if ((ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_GLOBAL_MODE && cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W2_INTERFACE_LOOPBACK)
#if (CLI_SUPPORT_INTERFACE_TO_INTERFACE == 1)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_VLAN_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_INTERFACE_LOOPBACK)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_ETH_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_INTERFACE_LOOPBACK)
#if (SYS_CPNT_LOOPBACK_IF_VISIBLE == TRUE)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_LOOPBACK_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_LOOPBACK_CMD_W2_INTERFACE_LOOPBACK)
#endif
#if (SYS_CPNT_CRAFT_PORT == TRUE)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_CRAFT_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_CRAFT_CMD_W2_INTERFACE_LOOPBACK)
#endif
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_ETH0_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_ETH0_CMD_W2_INTERFACE_LOOPBACK)
#endif
#if (SYS_CPNT_TRUNK_UI == TRUE)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_PCHANNEL_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_INTERFACE_LOOPBACK)
#endif
#endif
        )
    {
        ctrl_P->CMenu.loopback_id = (UI32_T)atoi((char*)arg[0]);
        ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_INTERFACE_LOOPBACK_MODE;

        if (NETCFG_PMGR_IP_CreateLoopbackInterface((UI32_T)atoi((char*)arg[0])) != NETCFG_TYPE_OK)
        {
            CLI_LIB_PrintStr("Fail to change interface type.\r\n");
            return CLI_NO_ERROR;
        }
    }
    else if (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_GLOBAL_MODE && cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_INTERFACE_LOOPBACK)
    {
        if (NETCFG_PMGR_IP_DeleteLoopbackInterface((UI32_T)atoi((char*)arg[0])) != NETCFG_TYPE_OK)
        {
            CLI_LIB_PrintStr("Fail to change interface type.\r\n");
            return CLI_NO_ERROR;
        }
    }
    else
    {
        return CLI_ERR_CMD_INVALID;
    }
#endif /* (SYS_CPNT_LOOPBACK_IF_VISIBLE == TRUE) */

    return CLI_NO_ERROR;
}


/* command : ip address */
UI32_T CLI_API_Loopback_Ip_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LOOPBACK_IF_VISIBLE == TRUE)
    UI32_T ret;
    UI32_T loopback_ifindex = 0;
    UI32_T uint_ipMask    = 0;
    UI8_T  byte_mask[SYS_ADPT_IPV4_ADDR_LEN] = {};
    NETCFG_TYPE_InetRifConfig_T rif_config;

    memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

    if (FALSE == IP_LIB_ConvertLoopbackIdToIfindex(ctrl_P->CMenu.loopback_id, &loopback_ifindex))
        CLI_LIB_PrintStr("No such Loopback interface.\r\n");

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_LOOPBACK_CMD_W2_IP_ADDRESS:
            memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
            rif_config.ifindex = loopback_ifindex;
            rif_config.ipv4_role = NETCFG_TYPE_MODE_PRIMARY; /* default */
            rif_config.row_status = VAL_netConfigStatus_2_createAndGo;

            if((arg[1]!=NULL) && CLI_LIB_AtoIp(arg[1], (UI8_T*)&uint_ipMask))
            {
                /* format: 192.168.1.1 255.255.255.0 */

                /* convert to byte array format */
                IP_LIB_UI32toArray(uint_ipMask, byte_mask);

                if(!IP_LIB_IsValidNetworkMask(byte_mask))
                {
                    CLI_LIB_PrintStr_1("Invalid Netmask address - %s\r\n", arg[1]);
                    return CLI_NO_ERROR;
                }

                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                                     arg[0],
                                                                     (L_INET_Addr_T *) &rif_config.addr,
                                                                     sizeof(rif_config.addr)))
                {
                    CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
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

/*  allow to set x.x.x.x/32 on an loopback interface
 */
#if 0
                case IP_LIB_INVALID_IP_SUBNET_BROADCAST_ADDR:
                    CLI_LIB_PrintStr("Invalid IP address. Can't be network b'cast IP.\r\n");
                    return CLI_NO_ERROR;
#endif
                default:
                    break;
            }

            ret = NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
            if(ret != NETCFG_TYPE_OK)
            {
                CLI_LIB_PrintStr_1("Failed to set IP address on Loopback interface %lu\r\n",
                            (unsigned long)ctrl_P->CMenu.loopback_id);
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_LOOPBACK_CMD_W3_NO_IP_ADDRESS:
            /* get loopback ip first*/
            memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
            rif_config.ifindex = loopback_ifindex;

            if(arg[0] == NULL)     /* no ip address */
            {
                if(NETCFG_TYPE_OK == NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config))
                {
                    rif_config.row_status = VAL_netConfigStatus_2_destroy;
                    if(NETCFG_TYPE_OK != NETCFG_PMGR_IP_SetInetRif(&rif_config,NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB))
                    {
                        CLI_LIB_PrintStr_1("Failed to destory IP address on Loopback interface %lu\r\n",
                            (unsigned long)ctrl_P->CMenu.loopback_id);
                    }
                 }
            }
            else
            {
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

                    /* convert to byte array format */
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
                    /* There is no matched address on this interface */
                    CLI_LIB_PrintStr_1("Can't find address on Loopback interface %lu\r\n",
                                (unsigned long)ctrl_P->CMenu.loopback_id);
                    return CLI_NO_ERROR;
                }
                rif_config.row_status = VAL_netConfigStatus_2_destroy;

                ret = NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
                if(ret != NETCFG_TYPE_OK)
                {
                    CLI_LIB_PrintStr_1("Failed to remove IP address on Loopback interface %lu\r\n",
                                (unsigned long)ctrl_P->CMenu.loopback_id);
                    return CLI_NO_ERROR;
                }
            }
            break;

        default:
            return CLI_ERR_CMD_UNRECOGNIZED;
    }
#endif /* (SYS_CPNT_LOOPBACK_IF_VISIBLE == TRUE) */

    return CLI_NO_ERROR;
}

