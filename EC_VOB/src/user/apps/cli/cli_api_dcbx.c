#include "cli_api.h"
#include "cli_api_dcbx.h"

#include "dcbx_pmgr.h"
#include "dcbx_type.h"

#if defined(FTTH_OKI)
#include "sys_cpnt.h"
#endif

#if (SYS_CPNT_PFC == TRUE)
#include "pfc_pmgr.h"
#endif
#if (SYS_CPNT_ETS == TRUE)
#include "ets_pmgr.h"
#endif


#define CLI_API_Dcbx_PrintHex(buf, len) do {     \
    UI32_T i;                                   \
    if (len > 0)                                 \
    {                                            \
        CLI_LIB_PrintStr_1("%02X", buf[0]);      \
        for (i = 1; i < len; i++)                \
            CLI_LIB_PrintStr_1("-%02X", buf[i]); \
    }                                            \
} while (0)


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Dcbx_eth
 *-------------------------------------------------------------------------
 * PURPOSE  : API for dcbx and no dcbx in ineterface ethernet
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Dcbx_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DCBX == TRUE)
    UI32_T i       = 0;
    BOOL_T status  = TRUE;
    UI32_T lport   = 0;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_DCBX:
            status = TRUE;
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_DCBX:
            status =  FALSE;
            break;

        default:
           return CLI_ERR_INTERNAL;
    }

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else if (DCBX_PMGR_SetPortStatus(lport, status)!=DCBX_TYPE_RETURN_OK)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to %s DCBX on Ethernet %s.\r\n", status == TRUE ? "enable" : "disable", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_3("Failed to %s DCBX on Ethernet %lu/%lu.\r\n", status == TRUE ? "enable" : "disable",
                                                                                (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
        }
    }
#endif /* #if (SYS_CPNT_DCBX == TRUE) */
    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Dcbx_pch
 *-------------------------------------------------------------------------
 * PURPOSE  : API for dcbx and no dcbx
 *            port status in interface pch.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Dcbx_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DCBX == TRUE)
    BOOL_T status  = TRUE;
    UI32_T ifindex = 0;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T verify_ret;

    if ((verify_ret = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_DCBX:
            status = TRUE;
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_DCBX:
            status = FALSE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (DCBX_PMGR_SetPortStatus(ifindex, status)!=DCBX_TYPE_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_2("Failed to %s DCBX on Trunk %lu.\r\n", status == TRUE ? "enable" : "disable", (unsigned long)verify_trunk_id);
#endif
    }
#endif /* #if (SYS_CPNT_DCBX == TRUE) */
    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Dcbx_eth
 *-------------------------------------------------------------------------
 * PURPOSE  : API for dcbx mode and no dcbx mode in ineterface ethernet
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_DcbxMode_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DCBX == TRUE)
    UI32_T i       = 0;
    UI32_T mode    = 0;
    UI32_T lport   = 0;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_DCBX_MODE:
            if(arg[0]!=NULL)
            {
                /**auto-down**/
                if ((*(arg[0]) == 'A' || *(arg[0]) == 'a')&&(*(arg[0]+5) == 'D' || *(arg[0]+5) == 'd'))
                     mode = DCBX_TYPE_PORT_MODE_AUTODOWN;
                /**auto-up**/
                else if ((*(arg[0]) == 'A' || *(arg[0]) == 'a')&&(*(arg[0]+5) == 'U' || *(arg[0]+5) == 'u'))
                    mode = DCBX_TYPE_PORT_MODE_AUTOUP;
                /**config-source**/
                else if(*(arg[0]) == 'C' || *(arg[0]) == 'c')
                    mode = DCBX_TYPE_PORT_MODE_CFGSRC;
                /** manual**/
                else if(*(arg[0]) == 'M' || *(arg[0]) == 'm')
                    mode = DCBX_TYPE_PORT_MODE_MANUAL;
                else
                    return CLI_ERR_INTERNAL;
            }
            else
                 return CLI_ERR_INTERNAL;
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_DCBX_MODE:
            mode =  DCBX_TYPE_PORT_MODE_MANUAL;
            break;

        default:
           return CLI_ERR_INTERNAL;
    }

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else if (DCBX_PMGR_SetPortMode(lport, mode)!=DCBX_TYPE_RETURN_OK)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to set DCBX port mode on Ethernet %s.\r\n", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to set DCBX port mode on Ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
        }
    }
#endif /* #if (SYS_CPNT_DCBX == TRUE) */
    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_DcbxMode_pch
 *-------------------------------------------------------------------------
 * PURPOSE  : API for dcbx mode and no dcbx mode
 *            port mode in interface pch.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_DcbxMode_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DCBX == TRUE)
    UI32_T mode    = 0;
    UI32_T ifindex = 0;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T verify_ret;

    if ((verify_ret = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_DCBX_MODE:
            if(arg[0]!=NULL)
            {
                /**auto-down**/
                if ((*(arg[0]) == 'A' || *(arg[0]) == 'a')&&(*(arg[0]+5) == 'D' || *(arg[0]+5) == 'd'))
                     mode = DCBX_TYPE_PORT_MODE_AUTODOWN;
                /**auto-up**/
                else if ((*(arg[0]) == 'A' || *(arg[0]) == 'a')&&(*(arg[0]+5) == 'U' || *(arg[0]+5) == 'u'))
                    mode = DCBX_TYPE_PORT_MODE_AUTOUP;
                /**config-source**/
                else if(*(arg[0]) == 'C' || *(arg[0]) == 'c')
                    mode = DCBX_TYPE_PORT_MODE_CFGSRC;
                /** manual**/
                else if(*(arg[0]) == 'M' || *(arg[0]) == 'm')
                    mode = DCBX_TYPE_PORT_MODE_MANUAL;
                else
                    return CLI_ERR_INTERNAL;
            }
            else
                 return CLI_ERR_INTERNAL;
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_DCBX_MODE:
            mode =  DCBX_TYPE_PORT_MODE_MANUAL;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (DCBX_PMGR_SetPortMode(ifindex, mode)!=DCBX_TYPE_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to set DCBX port mode on Trunk %lu.\r\n", (unsigned long)verify_trunk_id);
#endif
    }
#endif /* #if (SYS_CPNT_DCBX == TRUE) */
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_DCBX == TRUE)
static UI32_T show_one_dcbx_header(UI32_T line_num)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};

    CLI_LIB_PrintStr(" DCBX Port Configuration");
    PROCESS_MORE("\r\n");
    PROCESS_MORE("  Port     Status   Mode                ETS Willing PFC Willing\r\n");
    PROCESS_MORE("  -------- -------- ------------------- ----------- -----------\r\n");
    return line_num;
}

static UI32_T show_one_dcbx(UI32_T lport, UI32_T line_num)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};

    UI32_T  unit_id;
    UI32_T  port_id;
    UI32_T  trunk_id;
    BOOL_T  is_trunk;
    BOOL_T  port_status = FALSE;
    UI32_T  port_mode = DCBX_TYPE_PORT_MODE_MANUAL;
    BOOL_T  ets_willing_bit = FALSE;
    BOOL_T  pfc_willing_bit = FALSE;
#if (SYS_CPNT_PFC == TRUE)
    UI32_T  pfc_mode = 0;
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_TYPE_MODE_T ets_mode;
#endif

    switch (SWCTRL_POM_LogicalPortToUserPort(lport, &unit_id, &port_id, &trunk_id))
    {
        case SWCTRL_LPORT_NORMAL_PORT:
            is_trunk = FALSE;
            break;
        case SWCTRL_LPORT_TRUNK_PORT:
            is_trunk = TRUE;
            break;
        default:
            return JUMP_OUT_MORE;
    }

    DCBX_PMGR_GetPortStatus(lport, &port_status);
    DCBX_PMGR_GetPortMode(lport, &port_mode);

#if (SYS_CPNT_PFC == TRUE)
    PFC_PMGR_GetDataByField(lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &pfc_mode);
    if(((port_mode == DCBX_TYPE_PORT_MODE_CFGSRC) || (port_mode == DCBX_TYPE_PORT_MODE_AUTOUP)) &&
        (pfc_mode == PFC_TYPE_PMODE_AUTO))
    {
        pfc_willing_bit = TRUE;
    }
    else
#endif
    {
        pfc_willing_bit = FALSE;
    }

#if (SYS_CPNT_ETS == TRUE)
    ETS_PMGR_GetMode(lport, &ets_mode);
    if(((port_mode == DCBX_TYPE_PORT_MODE_CFGSRC) || (port_mode == DCBX_TYPE_PORT_MODE_AUTOUP))
       && (ets_mode == ETS_TYPE_MODE_AUTO)
      )
    {
        ets_willing_bit = TRUE;
    }
    else
#endif
    {
        ets_willing_bit = FALSE;
    }

    if(is_trunk)
    {
        CLI_LIB_PrintStr_1("  Trunk %-2lu ", (unsigned long)trunk_id);
    }
    else
    {
        CLI_LIB_PrintStr_2("  Eth %lu/%-2lu ", (unsigned long)unit_id, (unsigned long)port_id);
    }

    if(port_status)
    {
        CLI_LIB_PrintStr("Enabled  ");
    }
    else
    {
        CLI_LIB_PrintStr("Disabled ");
    }

    if(port_mode == DCBX_TYPE_PORT_MODE_AUTODOWN)
    {
        CLI_LIB_PrintStr("AutoDownstream      ");
    }
    else if(port_mode == DCBX_TYPE_PORT_MODE_AUTOUP)
    {
        CLI_LIB_PrintStr("AutoUpstream        ");
    }
    else if(port_mode == DCBX_TYPE_PORT_MODE_CFGSRC)
    {
        CLI_LIB_PrintStr("ConfigurationSource ");
    }
    else if(port_mode == DCBX_TYPE_PORT_MODE_MANUAL)
    {
        CLI_LIB_PrintStr("Manual              ");
    }

    if(ets_willing_bit)
    {
        CLI_LIB_PrintStr("Yes         ");
    }
    else
    {
        CLI_LIB_PrintStr("No          ");
    }

    if(pfc_willing_bit)
    {
        CLI_LIB_PrintStr("Yes");
    }
    else
    {
        CLI_LIB_PrintStr("No");
    }

    PROCESS_MORE("\r\n");
    return line_num;
}

#endif /* end of #if (SYS_CPNT_DCBX == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_LldpLocal
 *-------------------------------------------------------------------------
 * PURPOSE  : Show the information of LLDP local devices (global and interface)
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_Dcbx(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DCBX == TRUE)
    UI32_T line_num = 0;
    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T unit, i, trunk_id;
    UI32_T err_idx;
    UI32_T lport = 0;
    CLI_API_EthStatus_T verify_ret_e;
    CLI_API_TrunkStatus_T verify_ret_t;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char *s;
    char delemiters[2] = {0};

    delemiters[0] = ',';

    if(arg[0] == NULL)
    {
        line_num = show_one_dcbx_header(line_num);
        while (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
            if ((line_num = show_one_dcbx(lport, line_num)) ==  JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
            else
            {
                PROCESS_MORE("\r\n");
            }
        }
    }
    else if (arg[0][0] == 'e' || arg[0][0] == 'E')
    {
        line_num = show_one_dcbx_header(line_num);
        s = arg[1];

        /*get the unit*/
        unit = atoi(s);

        /*move the ptr to just after the slash */
        s = strchr(s, '/') + 1;

        while(1)
        {
            s =  CLI_LIB_Get_Token(s, Token, delemiters);

            if(CLI_LIB_CheckNullStr(Token))
            {
                if(s == 0)
                    break;
                else
                    continue;
            }
            else
            {
                CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx);

                for(i=lower_val; i<=upper_val; i++)
                {
                    if ((verify_ret_e = verify_ethernet(unit, i, &lport)) != CLI_API_ETH_OK)
                    {
                        display_ethernet_msg(verify_ret_e, unit, i);
                        continue;
                    }

                    if ((line_num = show_one_dcbx(lport, line_num)) ==  JUMP_OUT_MORE)
                    {
                        return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                        return CLI_EXIT_SESSION;
                    }
                    else
                    {
                        PROCESS_MORE("\r\n");
                    }
                }
            }
            if(s == 0)
                break;
            else
                memset(Token, 0, sizeof(Token));
        }
    }
    else if (arg[0][0] == 'p' || arg[0][0] == 'P')
    {
        trunk_id = atoi(arg[1]);
        if ((verify_ret_t = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
        {
            display_trunk_msg(verify_ret_t, trunk_id);
            return CLI_NO_ERROR;
        }

        line_num = show_one_dcbx_header(line_num);

        if((line_num = show_one_dcbx(lport, line_num)) ==  JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
        else
        {
            PROCESS_MORE("\r\n");
        }
    }
#endif /* #if (SYS_CPNT_DCBX == TRUE) */
    return CLI_NO_ERROR;
}
