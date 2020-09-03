#include "cli_api.h"
#include "swctrl_pom.h"

UI32_T CLI_API_Mac_Translate_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ITRI_MIM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;

    BOOL_T status, ret;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_MACTRANSLATE:
            status = TRUE;
            break;
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_MACTRANSLATE:
            status = FALSE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            if ((verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }

            ret = SWCTRL_PMGR_ITRI_MIM_SetStatus(lport, status);

            if (!ret)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(lport,name);
                  CLI_LIB_PrintStr_1("Failed to set MAC translation on ethernet %s.\r\n", name);
               }
#else
               CLI_LIB_PrintStr_2("Failed to set MAC translation on ethernet %lu/%lu.\r\n", verify_unit, verify_port);
#endif
            }
        } /* end of if (ctrl_P->CMenu.port_id_list) */
    } /* end of for i */
#endif /* (SYS_CPNT_ITRI_MIM == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Mac_Translate_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ITRI_MIM == TRUE)
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;

    BOOL_T status, ret;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_MACTRANSLATE:
            status = TRUE;
            break;
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_MACTRANSLATE:
            status = FALSE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    ret = SWCTRL_PMGR_ITRI_MIM_SetStatus(lport, status);

    if (!ret)
    {
       CLI_LIB_PrintStr_1("Failed to set MAC translation on trunk %lu.\r\n", verify_trunk_id);
    }
#endif /* (SYS_CPNT_ITRI_MIM == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Mac_Translate(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ITRI_MIM == TRUE)
    UI32_T line_num = 0;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};

    UI32_T unit, port, trunk_id;
    UI32_T lport = 0;
    BOOL_T status;

    PROCESS_MORE("Interface Status\r\n");
    PROCESS_MORE("--------- --------\r\n");

    while (SWCTRL_POM_ITRI_MIM_GetNextStatus(&lport, &status))
    {
        switch (SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
        {
            case SWCTRL_LPORT_NORMAL_PORT:
                sprintf(buff, "Eth %lu/%2lu ", unit, port);
                break;

            case SWCTRL_LPORT_TRUNK_PORT:
                sprintf(buff, "Trunk %2lu ", trunk_id);
                break;

            default:
                continue;
        }

        if (status)
        {
            strcat(buff, " Enabled ");
        }
        else
        {
            strcat(buff, " Disabled");
        }

        strcat(buff, "\r\n");
        PROCESS_MORE(buff);
    } /* end of while get next */
#endif /* (SYS_CPNT_ITRI_MIM == TRUE) */

    return CLI_NO_ERROR;
}

