#include <stdio.h>
#include <ctype.h>
#include "cli_api.h"
#include "cli_api_pvlan_ms.h"
#include "swctrl.h"

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
static BOOL_T fill_eth_lport_list(char *unit_port_str, UI8_T *port_list);
static BOOL_T fill_trunk_lport_list(char *trunk_str, UI8_T *port_list);
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

/* 2007-07-16 Eugene add for private VLAN traffic segmentation feature */
UI32_T CLI_API_Pvlan_MS(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)

    /* enable or disable private VLAN */
    if(arg[0] == NULL)
    {
        switch(cmd_idx)
        {
            case PRIVILEGE_CFG_GLOBAL_CMD_W1_TRAFFICSEGMENTATION:
                /* enable private VLAN */
                if(!SWCTRL_PMGR_EnablePrivateVlan())
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to enable private VLAN.\r\n");
#endif
                }
                break;

            case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_TRAFFICSEGMENTATION:
                /* disable private VLAN */
                if(!SWCTRL_PMGR_DisablePrivateVlan())
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to disable private VLAN.\r\n");
#endif
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }

        return CLI_NO_ERROR;
    }
    /* uplink-to-uplink
     */
    else if (strncmp(arg[0], "uplink-", strlen("uplink-")) == 0)
    {
        return CLI_API_Pvlan_MS_U2U(cmd_idx, arg + 1, ctrl_P);
    }
    /* add or remove uplink/downlink ports of private VLAN*/
    else
    {
        UI32_T session_id = 1;
        UI32_T arg_idx = 0;
        UI32_T up_down_flag = 0;    /* 0 invalid, 1 uplick, 2 downlink */
        UI32_T i = 0;
        UI8_T  uplink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
        UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
        UI8_T  start_pos = 0;
        BOOL_T ret;

        /* set uplink and downlink port list of private VLAN */
        memset(uplink_port_list, 0, sizeof(uplink_port_list));
        memset(downlink_port_list, 0, sizeof(downlink_port_list));

        /* session */
        if (arg[0][0] == 's' || arg[0][0] == 'S')
        {
            session_id = atoi(arg[1]);
            start_pos = 2;

            /* session range check */
            if ((session_id < 1) || (session_id > SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS))
            {
                CLI_LIB_PrintStr_1("Session %lu is not available.\r\n", (unsigned long)session_id);
                return CLI_NO_ERROR;
            }

            /* remove all uplink/downlink ports from the session */
            if ((cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_TRAFFICSEGMENTATION) && (arg[start_pos] == NULL))
            {
                if (!SWCTRL_POM_GetPrivateVlanBySessionId(session_id, uplink_port_list, downlink_port_list))
                {
                    CLI_LIB_PrintStr_1("Session %lu does not exist.\r\n", (unsigned long)session_id);

                    return CLI_NO_ERROR;
                }
                else if (!SWCTRL_PMGR_DestroyPrivateVlanSession(session_id, TRUE, TRUE))
                {
                    CLI_LIB_PrintStr_1("Failed to destroy session %lu.\r\n", (unsigned long)session_id);

                    return CLI_NO_ERROR;
                }

                return CLI_NO_ERROR;
            }
        }

        ret = TRUE;

        /* transform interfaces into one-bit port-list */
        for(arg_idx = start_pos; ret && arg[arg_idx] != NULL; arg_idx++)
        {
            /* uplink */
            if (arg[arg_idx][0] == 'u' || arg[arg_idx][0] == 'U')
            {
                up_down_flag = 1;
                arg_idx++;
            }
            /* downlink */
            else if (arg[arg_idx][0] == 'd' || arg[arg_idx][0] == 'D')
            {
                up_down_flag = 2;
                arg_idx++;
            }

            /* ethernet */
            if (arg[arg_idx][0] == 'e' || arg[arg_idx][0] == 'E')
            {
                if (up_down_flag == 1)
                    ret = fill_eth_lport_list(arg[++arg_idx], uplink_port_list);
                else if (up_down_flag == 2)
                    ret = fill_eth_lport_list(arg[++arg_idx], downlink_port_list);
            }

            /* port-channel */
            if (arg[arg_idx][0] == 'p' || arg[arg_idx][0] == 'P')
            {
                if (up_down_flag == 1)
                    ret = fill_trunk_lport_list(arg[++arg_idx], uplink_port_list);
                else if (up_down_flag == 2)
                    ret = fill_trunk_lport_list(arg[++arg_idx], downlink_port_list);
            }
        }

        if (!ret)
        {
            return CLI_NO_ERROR;
        }

        /* check */
        for(i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
        {
            if (uplink_port_list[i] & downlink_port_list[i])
            {
                CLI_LIB_PrintStr("Failed to set uplink/downlink ports of private VLAN due to the overlap of ports.\r\n");
                return CLI_NO_ERROR;
            }
        }

        /* action */
        switch(cmd_idx)
        {
            case PRIVILEGE_CFG_GLOBAL_CMD_W1_TRAFFICSEGMENTATION:
                /* add uplink and downlink ports of private VLAN */
                if(!SWCTRL_PMGR_SetPrivateVlanBySessionId(session_id, uplink_port_list, downlink_port_list))
                {
                    if (arg[start_pos] == NULL)
                    {
                        CLI_LIB_PrintStr_1("Failed to create session %lu.\r\n", (unsigned long)session_id);
                    }
                    else
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to add uplink/downlink ports of private VLAN.\r\n");
#endif
                    }
                }
                break;

            case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_TRAFFICSEGMENTATION:
                /* remove uplink and downlink ports of private VLAN */
                if(!SWCTRL_PMGR_DeletePrivateVlanPortlistBySessionId(session_id, uplink_port_list, downlink_port_list))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to remove uplink/downlink ports of private VLAN.\r\n");
#endif
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }

#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

    return CLI_NO_ERROR;
}

static BOOL_T fill_eth_lport_list(char *unit_port_str, UI8_T *port_list)
{
    UI32_T unit = atoi(unit_port_str);
    UI32_T port;
    UI32_T lower_val;
    UI32_T upper_val;
    UI32_T err_idx;
    UI32_T lport;
    char   *op_ptr;
    char   Token[CLI_DEF_MAX_BUFSIZE];
    char   delemiters[2] = {0};
    BOOL_T is_fail = FALSE;

    op_ptr = strchr(unit_port_str, '/') + 1;
    delemiters[0] = ',';

    do
    {
        memset(Token, 0, sizeof(Token));
        op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

        if(!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
            return FALSE;
        else
        {
            for(port = lower_val; port<=upper_val; port++)
            {
                CLI_API_EthStatus_T verify_ret;
                verify_ret = verify_ethernet(unit, port, &lport);

                switch(verify_ret)
                {
                    case CLI_API_ETH_OK:
                        port_list[(UI32_T)((lport-1)/8)] |= (1 << ( 7 - ((lport-1)%8)));
                        break;

                    default:
                        display_ethernet_msg(verify_ret, unit, port);
                        is_fail = TRUE;
                        break;
                }
            }
        }
    } while(op_ptr != 0 && !isspace(*op_ptr));

    if(is_fail)
        return FALSE;
    else
        return TRUE;
}

static BOOL_T fill_trunk_lport_list(char *trunk_str, UI8_T *port_list)
{
    UI32_T trunk_id;
    UI32_T lower_val;
    UI32_T upper_val;
    UI32_T err_idx;
    UI32_T lport;
    char   *op_ptr;
    char   Token[CLI_DEF_MAX_BUFSIZE];
    char   delemiters[2] = {0};
    BOOL_T is_fail = FALSE;

    op_ptr = trunk_str;
    delemiters[0] = ',';

    do
    {
        memset(Token, 0, sizeof(Token));
        op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

        if(!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
            return FALSE;
        else
        {
            for(trunk_id = lower_val; trunk_id<=upper_val; trunk_id++)
            {
                CLI_API_TrunkStatus_T verify_ret;
                verify_ret = verify_trunk(trunk_id, &lport);

                switch(verify_ret)
                {
                    case CLI_API_TRUNK_OK:
                        port_list[(UI32_T)((lport-1)/8)] |= (1 << ( 7 - ((lport-1)%8)));
                        break;

                    default:
                        display_trunk_msg(verify_ret, trunk_id);
                        is_fail = TRUE;
                        break;
                }
            }
        }
    } while(op_ptr != 0 && !isspace(*op_ptr));

    if(is_fail)
        return FALSE;
    else
        return TRUE;
}

UI32_T CLI_API_Show_Pvlan_MS(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
    UI32_T line_num = 0;
    UI32_T status = 0;
    UI32_T session_id = 0;
    char   buff[100] = {'\0'};
    char   up_buff[100] = {'\0'};
    char   down_buff[100] = {'\0'};
    BOOL_T show_one_session = FALSE;
    BOOL_T first_session = TRUE;

#if (SYS_CPNT_CLI_FILTERING == TRUE)
    if(arg[0]!=NULL)
    {
        if(arg[0][0]=='|')
        {
            switch(arg[1][0])
            {
                case 'b':
                case 'B':
                    ctrl_P->option_flag=CLI_API_OPTION_BEGIN;
                    break;
                case 'e':
                case 'E':
                    ctrl_P->option_flag=CLI_API_OPTION_EXCLUDE;
                    break;

                case 'i':
                case 'I':
                    ctrl_P->option_flag=CLI_API_OPTION_INCLUDE;
                    break;
                default:
                    return CLI_ERR_INTERNAL;
            }
            strcpy(ctrl_P->option_buf,arg[2]);
        }
    }

    if(arg[2]!=NULL)
    {
        if(arg[2][0]=='|')
        {
            switch(arg[3][0])
            {
                case 'b':
                case 'B':
                    ctrl_P->option_flag=CLI_API_OPTION_BEGIN;
                    break;
                case 'e':
                case 'E':
                    ctrl_P->option_flag=CLI_API_OPTION_EXCLUDE;
                    break;

                case 'i':
                case 'I':
                    ctrl_P->option_flag=CLI_API_OPTION_INCLUDE;
                    break;
                default:
                    return CLI_ERR_INTERNAL;
            }
            strcpy(ctrl_P->option_buf,arg[4]);
        }
    }
#endif /*#if (SYS_CPNT_CLI_FILTERING == TRUE)*/

    /* get private VLAN status */
    if (!SWCTRL_POM_GetPrivateVlanStatus(&status))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get private VLAN Status.\r\n");
#endif
        return CLI_NO_ERROR;
    }

    /* show private VLAN status */
    PROCESS_MORE("\r\n");
    sprintf( buff, " Private VLAN Status   :                  %s\r\n", (status == VAL_privateVlanStatus_enabled) ? "Enabled" : "Disabled");
    PROCESS_MORE(buff);

    /* get private VLAN mode */
    if (!SWCTRL_POM_GetPrivateVlanUplinkToUplinkStatus(&status))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get private VLAN Status.\r\n");
#endif
        return CLI_NO_ERROR;
    }

    /* show private VLAN mode */
    if (status == SYS_DFLT_TRAFFIC_SEG_UPLINK_TO_UPLINK_MODE_BLOCKING)
        sprintf( buff, " Uplink-to-Uplink Mode :                  %s\r\n", "Blocking");
    else if (status == SYS_DFLT_TRAFFIC_SEG_UPLINK_TO_UPLINK_MODE_FORWARDING)
        sprintf( buff, " Uplink-to-Uplink Mode :                  %s\r\n", "Forwarding");
    PROCESS_MORE(buff);

    if (arg[0] && (arg[0][0] == 's' || arg[0][0] == 'S'))
    {
        show_one_session = TRUE;
    }

    /* search by session */
    for(session_id = 1; session_id <= SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS; session_id++)
    {
        UI32_T lport = 0;
        UI32_T unit = 0;
        UI32_T port = 0;
        UI32_T trunk_id = 0;
        UI32_T up_length = 0;
        UI32_T down_length = 0;
        UI32_T i = 0;
        UI32_T uplink_port[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};
        UI32_T downlink_port[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};
        UI8_T  uplink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
        UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

        if (show_one_session)
        {
            session_id = atoi(arg[1]);

            /* session range check */
            if ((session_id < 1) || (session_id > SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS))
            {
                CLI_LIB_PrintStr_1("Session %lu is not available.\r\n", (unsigned long)session_id);
                break;
            }
        }

        /* get uplink/downlink port list */
        if (!SWCTRL_POM_GetPrivateVlanBySessionId(session_id, uplink_port_list, downlink_port_list))
        {
            if (show_one_session)
            {
                CLI_LIB_PrintStr_1("Session %lu does not exist.\r\n", (unsigned long)session_id);
                break;
            }
            else
            {
                continue;
            }
        }
        else
        {
            if (first_session)
            {
                PROCESS_MORE("\r\n");
                sprintf( buff, " Session   Uplink Ports                   Downlink Ports\r\n");
                PROCESS_MORE(buff);
                sprintf( buff, "--------- ------------------------------ -----------------------------\r\n");
                PROCESS_MORE(buff);
                first_session = FALSE;
            }

            /* more easy to show */
            /* one-bit uplink port list to array */
            for(lport = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER; lport<= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
            {
                if (uplink_port_list[(UI32_T)((lport-1)/8)] & (1<<(7-((lport-1)%8))))
                {
                    uplink_port[up_length] = lport;
                    up_length++;
                }
            }

            /* one-bit downlink port list to array */
            for(lport = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER; lport<= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
            {
                if (downlink_port_list[(UI32_T)((lport-1)/8)] & (1<<(7-((lport-1)%8))))
                {
                    downlink_port[down_length] = lport;
                    down_length++;
                }
            }

            /* show uplink/downlink port lists */
            while ((i <= up_length) || (i <= down_length))
            {
                SWCTRL_Lport_Type_T ret;

                /* uplink port list */
                if (uplink_port[i])
                {
                    ret = SWCTRL_POM_LogicalPortToUserPort(uplink_port[i], &unit, &port, &trunk_id);

                    switch(ret)
                    {
                        case SWCTRL_LPORT_NORMAL_PORT:
                        case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};

                            CLI_LIB_Ifindex_To_Name(uplink_port[i], name);
                            sprintf( up_buff, " Ethernet %20s", name);
                        }
#else
                            sprintf( up_buff, " Ethernet %2lu/%-2lu               ", (unsigned long)unit, (unsigned long)port);
#endif
                        break;

                        case SWCTRL_LPORT_TRUNK_PORT:
                            sprintf( up_buff, " Trunk %2lu                     ", (unsigned long)trunk_id);
                            break;

                        default:
                            sprintf( up_buff, " Invalid uplink port got from SWCTRL.\r\n");
                            PROCESS_MORE(up_buff);
                            return CLI_NO_ERROR;
                    }
                }
                /* uplink ports are more than downlink ports */
                else
                {
                    sprintf( up_buff, "                              ");
                }

                /* downlink port list */
                if (downlink_port[i])
                {
                    ret = SWCTRL_POM_LogicalPortToUserPort(downlink_port[i], &unit, &port, &trunk_id);

                    switch(ret)
                    {
                        case SWCTRL_LPORT_NORMAL_PORT:
                        case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};

                            CLI_LIB_Ifindex_To_Name(downlink_port[i], name);
                            sprintf( down_buff, "  Ethernet %-20s\r\n", name);
                        }
#else
                            sprintf( down_buff, "  Ethernet %2lu/%-2lu\r\n", (unsigned long)unit, (unsigned long)port);
#endif
                            break;

                        case SWCTRL_LPORT_TRUNK_PORT:
                            sprintf( down_buff, "  Trunk %2lu\r\n", (unsigned long)trunk_id);
                            break;

                        default:
                            sprintf( down_buff, "  Invalid downlink port got from SWCTRL.\r\n");
                            PROCESS_MORE(down_buff);
                            return CLI_NO_ERROR;
                    }
                }
                /* uplink ports are more than downlink ports */
                else
                {
                    sprintf( down_buff, "                              \r\n");
                }

                /* session id */
                if (!i)
                {
                    sprintf( buff, "   %2lu     ", (unsigned long)session_id);
                }
                else
                {
                    sprintf( buff, "          ");
                }

                /* concatenate */
                strcat( buff, up_buff);
                strcat( buff, down_buff);
                PROCESS_MORE(buff);

                i++;
            }

            if ((up_length == 0) && (down_length == 0))
                PROCESS_MORE("\r\n");
        }

        /* only show one session */
        if (show_one_session)
        {
            break;
        }
    }

    sprintf( buff, "\r\n");
    PROCESS_MORE(buff);

#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

   return CLI_NO_ERROR;
}

UI32_T CLI_API_Pvlan_MS_U2U(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_TRAFFICSEGMENTATION:
            /* set to blocking/forwarding mode */
            /* blocking */
            if (arg[0][0] == 'b' || arg[0][0] == 'B')
            {
                if (!SWCTRL_PMGR_EnablePrivateVlanUplinkToUplinkBlockingMode())
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set blocking mode to private VLAN.\r\n");
#endif
                }
            }
            /* forwarding */
            else if (arg[0][0] == 'f' || arg[0][0] == 'F')
            {
                if (!SWCTRL_PMGR_DisablePrivateVlanUplinkToUplinkBlockingMode())
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set forwarding mode to private VLAN.\r\n");
#endif
                }
            }
            else
                return CLI_ERR_INTERNAL;

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_TRAFFICSEGMENTATION:
            /* set to default mode */
            if (arg[0] == NULL)
            {
#if (SYS_DFLT_TRAFFIC_SEG_UPLINK_TO_UPLINK_MODE == SYS_DFLT_TRAFFIC_SEG_UPLINK_TO_UPLINK_MODE_BLOCKING)
                if (!SWCTRL_PMGR_EnablePrivateVlanUplinkToUplinkBlockingMode())
#elif (SYS_DFLT_TRAFFIC_SEG_UPLINK_TO_UPLINK_MODE == SYS_DFLT_TRAFFIC_SEG_UPLINK_TO_UPLINK_MODE_FORWARDING)
                if (!SWCTRL_PMGR_DisablePrivateVlanUplinkToUplinkBlockingMode())
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set default mode to private VLAN.\r\n");
#endif
                }
            }
            else
                return CLI_ERR_INTERNAL;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;


#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

   return CLI_NO_ERROR;
}
