/* MODULE NAME: cli_api_pppoe_ia.c
 * PURPOSE:
 *   Definitions of CLI APIs for PPPOE Intermediate Agent.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   03/30/09    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2009
 */

 /* INCLUDE FILE DECLARATIONS
  */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "cli_api.h"
#include "cli_lib.h"

#include "cli_api_pppoe_ia.h"
#include "swctrl.h"

#if (SYS_CPNT_PPPOE_IA == TRUE)
#include "pppoe_ia_pmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */
#define CLI_API_PPPOE_IA_CHK_LINE_NUM(ln_num)    \
                    switch (line_num)            \
                    {                            \
                    case EXIT_SESSION_MORE:      \
                        return CLI_EXIT_SESSION; \
                    case JUMP_OUT_MORE:          \
                        return CLI_NO_ERROR;     \
                    default:                     \
                        break;                   \
                    }

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if (SYS_CPNT_PPPOE_IA == TRUE)
static void Show_Info_Global(void);
static UI32_T Show_Info_Title(
    CLI_TASK_WorkingArea_T  *ctrl_P,
    UI32_T                  line_num,
    UI32_T                  step);
static UI32_T Show_Info_One(
    CLI_TASK_WorkingArea_T          *ctrl_P,
    UI32_T                          lport,
    UI32_T                          line_num,
    UI32_T                          step,
    PPPOE_IA_OM_PortOprCfgEntry_T   *pcfg_p,
    BOOL_T                          is_pcfg_ok);
static UI32_T Show_Statistics_One(CLI_TASK_WorkingArea_T *ctrl_P, UI32_T lport, UI32_T line_num);
#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* command: [no] pppoe intermediate-agent
 */
UI32_T CLI_API_PPPoE_IntermediateAgent(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    BOOL_T  is_enable;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_PPPOE_INTERMEDIATEAGENT:
        is_enable = TRUE;
        break;
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_PPPOE_INTERMEDIATEAGENT:
        is_enable = FALSE;
        break;
    default:
        return CLI_NO_ERROR;
    }

    if (FALSE == PPPOE_IA_PMGR_SetGlobalEnable(is_enable))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to configure PPPoE intermediate agent.\r\n");
#endif

    }
#endif

    return CLI_NO_ERROR;
}

/* command: [no] pppoe intermediate-agent port-enable
 */
UI32_T CLI_API_PPPoE_IntermediateAgent_PortEn_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    UI32_T  lport   = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port=0;
    UI32_T  verify_ret=0;
    UI32_T  i = 0;
    BOOL_T  is_enable;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_PPPOE_INTERMEDIATEAGENT_PORTENABLE:
        is_enable = TRUE;
        break;
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_PPPOE_INTERMEDIATEAGENT_PORTENABLE:
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

            if (FALSE == PPPOE_IA_PMGR_SetPortBoolDataByField(
                            lport, PPPOE_IA_TYPE_FLDID_PORT_ENABLE, is_enable))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2(
                    "Failed to configure PPPoE intermediate agent on port %lu/%lu.\r\n",
                    verify_unit, verify_port);
#endif
            }
        }
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_PPPoE_IntermediateAgent_PortEn_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    UI32_T                  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T                  lport;
    CLI_API_TrunkStatus_T   verify_ret;
    BOOL_T                  is_enable;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_PPPOE_INTERMEDIATEAGENT_PORTENABLE:
        is_enable = TRUE;
        break;
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_PPPOE_INTERMEDIATEAGENT_PORTENABLE:
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
        if (FALSE == PPPOE_IA_PMGR_SetPortBoolDataByField(
                        lport, PPPOE_IA_TYPE_FLDID_PORT_ENABLE, is_enable))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1(
                "Failed to configure PPPoE intermediate agent on trunk %lu.\r\n",
                verify_trunk_id);
#endif
        }
    }
#endif

    return CLI_NO_ERROR;
}

/* command: [no] pppoe intermediate-agent trust
 */
UI32_T CLI_API_PPPoE_IntermediateAgent_Trust_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    UI32_T  lport   = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port=0;
    UI32_T  verify_ret=0;
    UI32_T  i = 0;
    BOOL_T  is_enable;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_PPPOE_INTERMEDIATEAGENT_TRUST:
        is_enable = TRUE;
        break;
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_PPPOE_INTERMEDIATEAGENT_TRUST:
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

            if (FALSE == PPPOE_IA_PMGR_SetPortBoolDataByField(
                            lport, PPPOE_IA_TYPE_FLDID_PORT_TRUST, is_enable))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2(
                    "Failed to configure trusted mode on port %lu/%lu.\r\n",
                    verify_unit, verify_port);
#endif
            }
        }
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_PPPoE_IntermediateAgent_Trust_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    UI32_T                  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T                  lport;
    CLI_API_TrunkStatus_T   verify_ret;
    BOOL_T                  is_enable;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_PPPOE_INTERMEDIATEAGENT_TRUST:
        is_enable = TRUE;
        break;
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_PPPOE_INTERMEDIATEAGENT_TRUST:
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
        if (FALSE == PPPOE_IA_PMGR_SetPortBoolDataByField(
                        lport, PPPOE_IA_TYPE_FLDID_PORT_TRUST, is_enable))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1(
                "Failed to configure trusted mode on trunk %lu.\r\n",
                verify_trunk_id);
#endif
        }
    }
#endif

    return CLI_NO_ERROR;
}

/* command: [no] pppoe intermediate-agent vendor-tag strip
 */
UI32_T CLI_API_PPPoE_IntermediateAgent_VTag_Strip_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    UI32_T  lport   = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port=0;
    UI32_T  verify_ret=0;
    UI32_T  i = 0;
    BOOL_T  is_enable;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_PPPOE_INTERMEDIATEAGENT_VENDORTAG_STRIP:
        is_enable = TRUE;
        break;
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_NO_PPPOE_INTERMEDIATEAGENT_VENDORTAG_STRIP:
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

            if (FALSE == PPPOE_IA_PMGR_SetPortBoolDataByField(
                            lport, PPPOE_IA_TYPE_FLDID_PORT_STRIP_VENDOR, is_enable))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2(
                    "Failed to configure vendor-tag stripping on port %lu/%lu.\r\n",
                    verify_unit, verify_port);
#endif
            }
        }
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_PPPoE_IntermediateAgent_VTag_Strip_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    UI32_T                  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T                  lport;
    CLI_API_TrunkStatus_T   verify_ret;
    BOOL_T                  is_enable;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_PPPOE_INTERMEDIATEAGENT_VENDORTAG_STRIP:
        is_enable = TRUE;
        break;
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W5_NO_PPPOE_INTERMEDIATEAGENT_VENDORTAG_STRIP:
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
        if (FALSE == PPPOE_IA_PMGR_SetPortBoolDataByField(
                        lport, PPPOE_IA_TYPE_FLDID_PORT_STRIP_VENDOR, is_enable))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1(
                "Failed to configure vendor-tag stripping on trunk %lu.\r\n",
                verify_trunk_id);
#endif
        }
    }
#endif

    return CLI_NO_ERROR;
}

/* command: [no] pppoe intermediate-agent format-type
 *          {access-node-identifier | generic-error-message}
 */
UI32_T CLI_API_PPPoE_IntermediateAgent_FmtType(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    char    *str_p  = NULL;
    UI32_T  str_len = 0;
    UI32_T  fld_id  = PPPOE_IA_TYPE_FLDID_GLOBAL_ACC_NODE_ID;

    if (NULL == arg[0])
    {
        return CLI_NO_ERROR;
    }

    switch (arg[0][0])
    {
    case 'a':
    case 'A':
        break;
    case 'g':
    case 'G':
        fld_id  = PPPOE_IA_TYPE_FLDID_GLOBAL_GEN_ERMSG;
        break;
    default:
        return CLI_NO_ERROR;
    }

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_PPPOE_INTERMEDIATEAGENT_FORMATTYPE:
        str_len = strlen(arg[1]);
        str_p   = arg[1];
        break;
    case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_PPPOE_INTERMEDIATEAGENT_FORMATTYPE:
        break;
    default:
        return CLI_NO_ERROR;
    }

    if (FALSE == PPPOE_IA_PMGR_SetGlobalAdmStrDataByField(
                    fld_id, (UI8_T *) str_p, str_len))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        if (PPPOE_IA_TYPE_FLDID_GLOBAL_ACC_NODE_ID == fld_id)
        {
            CLI_LIB_PrintStr("Failed to configure access node identifier.\r\n");
        }
        else
        {
            CLI_LIB_PrintStr("Failed to configure generic error message.\r\n");
        }
#endif

    }
#endif

    return CLI_NO_ERROR;
}

/* command: [no] pppoe intermediate-agent port-format-type
 *          {circuit-id | remote-id | remote-id-delimiter {enable | ascii}}
 */
UI32_T CLI_API_PPPoE_IntermediateAgent_PortFmtType_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    char    *str_p  = NULL;
    UI32_T  str_len = 0;
    UI32_T  fld_id  = PPPOE_IA_TYPE_FLDID_PORT_CIRCUIT_ID;
    UI32_T  lport   = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port=0;
    UI32_T  verify_ret=0;
    UI32_T  i = 0, rid_dascii = PPPOE_IA_TYPE_DFLT_RID_DASCII;
    BOOL_T  is_no_form = FALSE;

    if (NULL == arg[0])
    {
        return CLI_NO_ERROR;
    }

    switch (arg[0][0])
    {
    case 'c':
    case 'C':
        break;
    case 'r':
    case 'R':
        fld_id  = PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID;
        if (NULL != arg[1])
        {
            if (strlen(arg[0]) > strlen("remote-id"))
            {
                if ((arg[1][0] == 'e') || (arg[1][0] == 'E'))
                {
                    fld_id  = PPPOE_IA_TYPE_FLDID_PORT_RID_DELIM;
                }
                else
                {
                    fld_id  = PPPOE_IA_TYPE_FLDID_PORT_RID_DASCII;
                    rid_dascii = atoi(arg[1]);
                }
            }
        }
        else
        {
            if (strlen(arg[0]) > strlen("remote-id"))
            {
                fld_id  = PPPOE_IA_TYPE_FLDID_PORT_RID_DASCII;
            }
        }
        break;
    default:
        return CLI_NO_ERROR;
    }

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_PPPOE_INTERMEDIATEAGENT_PORTFORMATTYPE:
        str_len = strlen(arg[1]);
        str_p   = arg[1];
        break;
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_PPPOE_INTERMEDIATEAGENT_PORTFORMATTYPE:
        is_no_form = TRUE;
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

            switch (fld_id)
            {
#if (SYS_CPNT_PPPOE_IA_REMOTE_ID_ENHANCE == TRUE)
            case PPPOE_IA_TYPE_FLDID_PORT_RID_DASCII:
                if (FALSE == PPPOE_IA_PMGR_SetPortUi32DataByField(
                                lport, fld_id, rid_dascii))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr_2("Failed to configure ASCII code of delimiter on port %lu/%lu.\r\n",
                        verify_unit, verify_port);
    #endif
                }
                break;
            case PPPOE_IA_TYPE_FLDID_PORT_RID_DELIM:
                if (FALSE == PPPOE_IA_PMGR_SetPortBoolDataByField(
                                lport, fld_id, !is_no_form))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr_2("Failed to configure delimiter on port %lu/%lu.\r\n",
                        verify_unit, verify_port);
    #endif
                }
                break;
#endif /* #if (SYS_CPNT_PPPOE_IA_REMOTE_ID_ENHANCE == TRUE) */
            case PPPOE_IA_TYPE_FLDID_PORT_CIRCUIT_ID:
            case PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID:

                if (FALSE == PPPOE_IA_PMGR_SetPortAdmStrDataByField(
                                lport, fld_id, (UI8_T *) str_p, str_len))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    if (PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID == fld_id)
                    {
                        CLI_LIB_PrintStr_2("Failed to configure remote-id on port %lu/%lu.\r\n",
                            verify_unit, verify_port);
                    }
                    else
                    {
                        CLI_LIB_PrintStr_2("Failed to configure circiut-id on port %lu/%lu.\r\n",
                            verify_unit, verify_port);
                    }
    #endif
                }

            default:
                break;
            } /* switch (fld_id) */
        }
    }
#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_PPPoE_IntermediateAgent_PortFmtType_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    char                    *str_p  = NULL;
    UI32_T                  str_len = 0;
    UI32_T                  fld_id  = PPPOE_IA_TYPE_FLDID_PORT_CIRCUIT_ID;
    UI32_T                  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T                  lport;
    UI32_T                  rid_dascii = PPPOE_IA_TYPE_DFLT_RID_DASCII;
    CLI_API_TrunkStatus_T   verify_ret;
    BOOL_T                  is_no_form = FALSE;

    if (NULL == arg[0])
    {
        return CLI_NO_ERROR;
    }

    switch (arg[0][0])
    {
    case 'c':
    case 'C':
        break;
    case 'r':
    case 'R':
        fld_id  = PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID;
        if (NULL != arg[1])
        {
            if (strlen(arg[0]) > strlen("remote-id"))
            {
                if ((arg[1][0] == 'e') || (arg[1][0] == 'E'))
                {
                    fld_id  = PPPOE_IA_TYPE_FLDID_PORT_RID_DELIM;
                }
                else
                {
                    fld_id  = PPPOE_IA_TYPE_FLDID_PORT_RID_DASCII;
                    rid_dascii = atoi(arg[1]);
                }
            }
        }
        else
        {
            if (strlen(arg[0]) > strlen("remote-id"))
            {
                fld_id  = PPPOE_IA_TYPE_FLDID_PORT_RID_DASCII;
            }
        }
        break;
    default:
        return CLI_NO_ERROR;
    }

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_PPPOE_INTERMEDIATEAGENT_PORTFORMATTYPE:
        str_len = strlen(arg[1]);
        str_p   = arg[1];
        break;
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_PPPOE_INTERMEDIATEAGENT_PORTFORMATTYPE:
        is_no_form = TRUE;
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
        switch (fld_id)
        {
        case PPPOE_IA_TYPE_FLDID_PORT_RID_DASCII:
            if (FALSE == PPPOE_IA_PMGR_SetPortUi32DataByField(
                            lport, fld_id, rid_dascii))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to configure ASCII code of delimiter on trunk %lu.\r\n",
                    verify_trunk_id);
#endif
            }
            break;
        case PPPOE_IA_TYPE_FLDID_PORT_RID_DELIM:
            if (FALSE == PPPOE_IA_PMGR_SetPortBoolDataByField(
                            lport, fld_id, !is_no_form))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to configure delimiter on trunk %lu.\r\n",
                    verify_trunk_id);
#endif
            }
            break;
        case PPPOE_IA_TYPE_FLDID_PORT_CIRCUIT_ID:
        case PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID:

            if (FALSE == PPPOE_IA_PMGR_SetPortAdmStrDataByField(
                            lport, fld_id, (UI8_T *) str_p, str_len))
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                if (PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID == fld_id)
                {
                    CLI_LIB_PrintStr_1("Failed to configure remote-id on trunk %lu.\r\n",
                        verify_trunk_id);
                }
                else
                {
                    CLI_LIB_PrintStr_1("Failed to configure circiut-id on trunk %lu.\r\n",
                        verify_trunk_id);
                }
    #endif
            }

        default:
            break;
        } /* switch (fld_id) */
    }
#endif

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_PPPOE_IA == TRUE)

static void Show_Info_Global(void)
{
    BOOL_T  global_en;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char    *en_str[] = {"Disabled", "Enabled"};
    char    *en_p;

    PPPOE_IA_PMGR_GetGlobalEnable(&global_en);
    en_p = (TRUE == global_en)?en_str[1]:en_str[0];
    CLI_LIB_PrintStr_2(
        "%-53s : %s\r\n", "PPPoE Intermediate Agent Global Status", en_p);

    PPPOE_IA_PMGR_GetAccessNodeId(FALSE, (UI8_T *) buff);
    CLI_LIB_PrintStr_2(
        "%-53s :\r\n %s\r\n", "PPPoE Intermediate Agent Admin Access Node Identifier", buff);

    PPPOE_IA_PMGR_GetAccessNodeId(TRUE, (UI8_T *) buff);
    CLI_LIB_PrintStr_2(
        "%-53s :\r\n %s\r\n", "PPPoE Intermediate Agent Oper Access Node Identifier", buff);

    PPPOE_IA_PMGR_GetGenericErrMsg(FALSE, (UI8_T *) buff);
    CLI_LIB_PrintStr_2(
        "%-53s :\r\n %s\r\n", "PPPoE Intermediate Agent Admin Generic Error Message", buff);

    PPPOE_IA_PMGR_GetGenericErrMsg(TRUE, (UI8_T *) buff);
    CLI_LIB_PrintStr_2(
        "%-53s :\r\n %s\r\n", "PPPoE Intermediate Agent Oper Generic Error Message", buff);
}

static UI32_T Show_Info_Title(
    CLI_TASK_WorkingArea_T  *ctrl_P,
    UI32_T                  line_num,
    UI32_T                  step)
{
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

    if (step == 0)
    {
        sprintf(buff, "%-9s %-8s %-7s %-16s %-16s %-17s\r\n",
                      "Interface", "PPPoE IA", "Trusted",
                      "Vendor-Tag Strip", "Admin Circuit-ID", "Admin Remote-ID");
        PROCESS_MORE_FUNC(buff);

        sprintf(buff, "%-9s %-8s %-7s %-16s %-16s %-17s\r\n",
                      "---------", "--------", "-------",
                      "----------------", "------------", "-----------------");
        PROCESS_MORE_FUNC(buff);
    }
    else
    {
        char    *rid_delim_tag_p = "", *rid_dascii_tag_p= "", *rid_sep_tag_p="";

#if (SYS_CPNT_PPPOE_IA_REMOTE_ID_ENHANCE == TRUE)
        rid_delim_tag_p = "R-ID Delimiter";
        rid_dascii_tag_p= "Delimiter ASCII";
        rid_sep_tag_p   = "----------------";
#endif

        sprintf(buff, "%-9s %-16s %-16s %-16s %-17s\r\n",
                      "", rid_delim_tag_p, rid_dascii_tag_p,
                      "Oper Circuit-ID", "Oper Remote-ID");
        PROCESS_MORE_FUNC(buff);

        sprintf(buff, "%-9s %-16s %-16s %-16s %-17s\r\n",
                      "", rid_sep_tag_p, rid_sep_tag_p,
                      "----------------", "-----------------");
        PROCESS_MORE_FUNC(buff);
    }

    return line_num;
}

static char *cli_api_pppoe_ia_copy_str_by_len(
    char    *src_p,
    UI8_T   src_len,
    UI8_T   src_beg,
    char    *dst_p,
    UI8_T   dst_len)
{
    char   *ret_p = NULL;
    UI8_T   copy_len;

    if ((NULL != src_p) && (NULL != dst_p))
    {
        if (src_len > src_beg)
        {
            if ((src_len - src_beg) > dst_len)
            {
                copy_len = dst_len;
            }
            else
            {
                copy_len = src_len - src_beg;
            }

            memcpy(dst_p, src_p+src_beg, copy_len);
            dst_p[copy_len]= '\0';
        }
        else
        {
            dst_p[0] = '\0';
        }
        ret_p = dst_p;
    }

    return ret_p;
}

static UI32_T Show_Info_One(
    CLI_TASK_WorkingArea_T          *ctrl_P,
    UI32_T                          lport,
    UI32_T                          line_num,
    UI32_T                          step,
    PPPOE_IA_OM_PortOprCfgEntry_T   *pcfg_p,
    BOOL_T                          is_pcfg_ok)
{
#define RID_MAX_LEN_ONE_ROW     17
#define RID_MAX_LOOP            ((PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN+RID_MAX_LEN_ONE_ROW-1)/RID_MAX_LEN_ONE_ROW)

    UI32_T                          unit, port, trunk_id;
    UI32_T                          loop_idx, name_beg_rid =0;
    SWCTRL_Lport_Type_T             port_type;
    char                            buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char                            port_str[12];
    char                            *en_str[] = {"No", "Yes"};
    char                            *en_p, *trust_p, *strip_p;
    char                            tmp_rid_buf[RID_MAX_LEN_ONE_ROW+1];

    port_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    if (  (port_type == SWCTRL_LPORT_TRUNK_PORT)
        ||(port_type == SWCTRL_LPORT_NORMAL_PORT)
       )
    {
        if (FALSE == is_pcfg_ok)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            if (port_type == SWCTRL_LPORT_NORMAL_PORT)
            {
                sprintf(buff, "Failed to display PPPoE intermediate agent information on port %lu/%lu.\r\n",
                    unit, port);
            }
            else
            {
                sprintf(buff, "Failed to display PPPoE intermediate agent information on trunk %lu.\r\n",
                    trunk_id);
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
        sprintf(port_str, "Eth %lu/%lu", unit, port);
    }
    else
    {
        sprintf(port_str, "Trunk %lu", trunk_id);
    }

    if (step == 0)
    {
        en_p    = (TRUE == pcfg_p->adm_cfg.is_enable)?en_str[1]:en_str[0];
        trust_p = (TRUE == pcfg_p->adm_cfg.is_trust)?en_str[1]:en_str[0];
        strip_p = (TRUE == pcfg_p->adm_cfg.is_strip_vtag)?en_str[1]:en_str[0];

        for (loop_idx =0; loop_idx < RID_MAX_LOOP; loop_idx++)
        {
            cli_api_pppoe_ia_copy_str_by_len(
                (char *)pcfg_p->adm_cfg.remote_id, pcfg_p->adm_cfg.remote_id_len,
                name_beg_rid, tmp_rid_buf, RID_MAX_LEN_ONE_ROW);

            if (loop_idx == 0)
            {
                sprintf(buff, "%-9s %-8s %-7s %-16s %-16s %-17s\r\n",
                                port_str, en_p, trust_p, strip_p,
                                pcfg_p->adm_cfg.circuit_id, tmp_rid_buf);
            }
            else
            {
                sprintf(buff, "%-9s %-8s %-7s %-16s %-16s %-17s\r\n",
                                "", "", "", "", "", tmp_rid_buf);
            }
            PROCESS_MORE_FUNC(buff);

            name_beg_rid += RID_MAX_LEN_ONE_ROW;
            if (pcfg_p->adm_cfg.remote_id_len <= name_beg_rid)
                break;
        }
    }
    else
    {
        char    *rid_delim_en_p = "", rid_dascii_val[16] = {0};

#if (SYS_CPNT_PPPOE_IA_REMOTE_ID_ENHANCE == TRUE)
        rid_delim_en_p = (TRUE == pcfg_p->adm_cfg.is_rid_delim_en)?en_str[1]:en_str[0];
        sprintf(rid_dascii_val, "%d", pcfg_p->adm_cfg.rid_delim_ascii);
#endif
        for (loop_idx =0; loop_idx < RID_MAX_LOOP; loop_idx++)
        {
            cli_api_pppoe_ia_copy_str_by_len(
                (char *)pcfg_p->opr_rmid, pcfg_p->opr_rmid_len,
                name_beg_rid, tmp_rid_buf, RID_MAX_LEN_ONE_ROW);

            if (loop_idx == 0)
            {
                sprintf(buff, "%-9s %-16s %16s %-16s %-17s\r\n",
                                "", rid_delim_en_p, rid_dascii_val, pcfg_p->opr_ccid, tmp_rid_buf);
            }
            else
            {
                sprintf(buff, "%-9s %-16s %16s %-16s %-17s\r\n",
                                "", "", "", "", tmp_rid_buf);
            }
            PROCESS_MORE_FUNC(buff);

            name_beg_rid += RID_MAX_LEN_ONE_ROW;
            if (pcfg_p->opr_rmid_len <= name_beg_rid)
                break;
        }
    }

    return line_num;

/* Sample:
 * Interface PPPoE IA Trusted Vendor-Tag Strip Admin Circuit-ID Admin Remote-ID
 * --------- -------- ------- ---------------- ---------------- -----------------
 *         9        8       7               16               16                17
 * Eth 1/1   Yes      No      No
 *           R-ID Delimiter   Delimiter ASCII  Oper Circuit-ID  Oper Remote-ID
 *           ---------------- ---------------- ---------------- -----------------
 *                         16               16               16                17
 *           No                             35 1/1:vid          00-00-00-00-00-01
 */
}
#endif  /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

/* command: show pppoe intermediate-agent info [interface [eth|pch]]
 */
UI32_T CLI_API_PPPoE_Show_IntermediateAgent_Info(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    UI32_T  ifindex;
    UI32_T  line_num = 0;
    UI32_T  i        = 0;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    UI32_T  verify_trunk_id = 0;
    UI32_T  max_port_num;
    UI32_T  current_max_unit = 0;
    UI32_T  j;
    CLI_API_EthStatus_T     verify_ret_e;
    CLI_API_TrunkStatus_T   verify_ret_t;
    BOOL_T                  is_pcfg_ok;
    PPPOE_IA_OM_PortOprCfgEntry_T   pcfg;

#if (SYS_CPNT_STACKING == TRUE)
    current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
    current_max_unit = 1;
#endif

    if (arg[0] == NULL)
    {
        Show_Info_Global();
    }
    else
    {
        if (arg[1] == NULL)
        {
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

                    is_pcfg_ok =  PPPOE_IA_PMGR_GetPortOprCfgEntry(ifindex, &pcfg);

                    line_num = Show_Info_Title(ctrl_P, line_num, 0);
                    CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);

                    line_num = Show_Info_One(ctrl_P, ifindex, line_num, 0, &pcfg, is_pcfg_ok);
                    CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);

                    line_num = Show_Info_Title(ctrl_P, line_num, 1);
                    CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);

                    line_num = Show_Info_One(ctrl_P, ifindex, line_num, 1, &pcfg, is_pcfg_ok);
                    CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);
                }
            }/*end of unit loop*/

            /*trunk*/
            while (TRK_PMGR_GetNextTrunkId(&verify_trunk_id))
            {
                if ((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
                {
                    continue;
                }
                is_pcfg_ok =  PPPOE_IA_PMGR_GetPortOprCfgEntry(ifindex, &pcfg);

                line_num = Show_Info_Title(ctrl_P, line_num, 0);
                CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);

                line_num = Show_Info_One(ctrl_P, ifindex, line_num, 0, &pcfg, is_pcfg_ok);
                CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);

                line_num = Show_Info_Title(ctrl_P, line_num, 1);
                CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);

                line_num = Show_Info_One(ctrl_P, ifindex, line_num, 1, &pcfg, is_pcfg_ok);
                CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);
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
                }
                else
                {
                    is_pcfg_ok =  PPPOE_IA_PMGR_GetPortOprCfgEntry(ifindex, &pcfg);

                    line_num = Show_Info_Title(ctrl_P, line_num, 0);
                    CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);

                    line_num = Show_Info_One(ctrl_P, ifindex, line_num, 0, &pcfg, is_pcfg_ok);
                    CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);

                    line_num = Show_Info_Title(ctrl_P, line_num, 1);
                    CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);

                    line_num = Show_Info_One(ctrl_P, ifindex, line_num, 1, &pcfg, is_pcfg_ok);
                    CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);
                }
                break;

            case 'p':
            case 'P':
                verify_trunk_id = atoi(arg[2]);
                if ((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(verify_ret_t, verify_trunk_id);
                }
                else
                {
                    is_pcfg_ok =  PPPOE_IA_PMGR_GetPortOprCfgEntry(ifindex, &pcfg);

                    line_num = Show_Info_Title(ctrl_P, line_num, 0);
                    CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);

                    line_num = Show_Info_One(ctrl_P, ifindex, line_num, 0, &pcfg, is_pcfg_ok);
                    CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);

                    line_num = Show_Info_Title(ctrl_P, line_num, 1);
                    CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);

                    line_num = Show_Info_One(ctrl_P, ifindex, line_num, 1, &pcfg, is_pcfg_ok);
                    CLI_API_PPPOE_IA_CHK_LINE_NUM(line_num);
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
            }
        }
    }
#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_PPPOE_IA == TRUE)
static UI32_T Show_Statistics_One(
    CLI_TASK_WorkingArea_T *ctrl_P,
    UI32_T  lport,
    UI32_T  line_num)
{
    UI32_T                      unit, port, trunk_id;
    SWCTRL_Lport_Type_T         port_type;
    PPPOE_IA_OM_PortStsEntry_T  psts;
    char                        buff[CLI_DEF_MAX_BUFSIZE] = {0};

    port_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    if (  (port_type == SWCTRL_LPORT_TRUNK_PORT)
        ||(port_type == SWCTRL_LPORT_NORMAL_PORT)
       )
    {
        if (FALSE == PPPOE_IA_PMGR_GetPortStatisticsEntry(
                        lport, &psts))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            if (port_type == SWCTRL_LPORT_NORMAL_PORT)
            {
                sprintf(buff, "Failed to display PPPoE intermediate agent statistics on port %lu/%lu.\r\n",
                    unit, port);
            }
            else
            {
                sprintf(buff, "Failed to display PPPoE intermediate agent statistics on trunk %lu.\r\n",
                    trunk_id);
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
        sprintf(buff, "Eth %lu/%lu statistics\r\n", unit, port);
    }
    else
    {
        sprintf(buff, "Trunk %lu statistics\r\n", trunk_id);
    }
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "-----------------------------------------------------------------------------\r\n");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, " Received : %10s %10s %10s %10s %10s %10s\r\n",
                    "All", "PADI", "PADO", "PADR", "PADS", "PADT");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "            %10s %10s %10s %10s %10s %10s\r\n",
                    "----------", "----------", "----------",
                    "----------", "----------", "----------");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "            %10lu %10lu %10lu %10lu %10lu %10lu\r\n",
                    psts.all, psts.padi, psts.pado, psts.padr, psts.pads, psts.padt);
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "\r\n");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, " Dropped  : %23s  %25s  %9s\r\n",
                    "Response from untrusted", "Request towards untrusted",
                    "Malformed");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "            %23s  %25s  %9s\r\n",
                    "-----------------------", "-------------------------",
                    "---------");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "            %23lu  %25lu  %9lu\r\n",
                    psts.rep_untrust, psts.req_untrust, psts.malform);
    PROCESS_MORE_FUNC(buff);
    return line_num;

/* Sample:
 * Eth 1/1 Statistics
 * Trunk 1 Statistics
 * -----------------------------------------------------------------------------
 *  Received :        All       PADI       PADO       PADR       PADS       PADT
 *             ---------- ---------- ---------- ---------- ---------- ----------
 *                      0          0          0          0          0          0
 *  Dropped  : Response from untrusted  Request towards untrusted  Malformed
 *             -----------------------  -------------------------  ---------
 *                                   0                          0          0
 */
}
#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

/* command: show pppoe intermediate-agent statistics interface [eth|pch]
 */
UI32_T CLI_API_PPPoE_Show_IntermediateAgent_Statistics(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    UI32_T  ifindex;
    UI32_T  line_num = 0;
    UI32_T  i        = 0;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    UI32_T  verify_trunk_id = 0;
    UI32_T  max_port_num;
    UI32_T  current_max_unit = 0;
    UI32_T  j;
    CLI_API_EthStatus_T     verify_ret_e;
    CLI_API_TrunkStatus_T   verify_ret_t;

#if (SYS_CPNT_STACKING == TRUE)
    current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
    current_max_unit = 1;
#endif

    if (arg[0] == NULL)
    {
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
                if ((line_num = Show_Statistics_One(ctrl_P, ifindex, line_num)) == EXIT_SESSION_MORE)
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
            if ((line_num = Show_Statistics_One(ctrl_P, ifindex, line_num)) == EXIT_SESSION_MORE)
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
        switch(arg[0][0])
        {
        case 'e':
        case 'E':
            verify_unit = atoi(arg[1]);
            verify_port = atoi(strchr(arg[1], '/') + 1);

            if ((verify_ret_e = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
            }
            else
            {
                Show_Statistics_One(ctrl_P, ifindex, line_num);
            }
            break;

        case 'p':
        case 'P':
            verify_trunk_id = atoi(arg[1]);
            if ((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret_t, verify_trunk_id);
            }
            else
            {
                Show_Statistics_One(ctrl_P, ifindex, line_num);
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
        }
    }
#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

    return CLI_NO_ERROR;
}

/* command: clear pppoe intermediate-agent statistics interface [eth|pch]
 */
UI32_T CLI_API_PPPoE_Clear_IntermediateAgent_Statistics(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
        UI32_T  ifindex;
        UI32_T  verify_unit;
        UI32_T  verify_port;
        UI32_T  verify_trunk_id;
        UI32_T  current_max_unit = 0;
        CLI_API_EthStatus_T     verify_ret_e;
        CLI_API_TrunkStatus_T   verify_ret_t;

#if (SYS_CPNT_STACKING == TRUE)
        current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
        current_max_unit = 1;
#endif

        if (arg[0] == NULL)
        {
            PPPOE_IA_PMGR_ClearPortStatistics(0);
        }
        else
        {
            switch(arg[0][0])
            {
            case 'e':
            case 'E':
                verify_unit = atoi(arg[1]);
                verify_port = atoi(strchr(arg[1], '/') + 1);

                if ((verify_ret_e = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
                {
                    display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                }
                else
                {
                    if (FALSE == PPPOE_IA_PMGR_ClearPortStatistics(ifindex))
                    {
                        CLI_LIB_PrintStr_2(
                            "Failed to clear PPPoE intermediate statistics on port %lu/%lu.\r\n",
                            verify_unit, verify_port);
                    }
                }
                break;

            case 'p':
            case 'P':
                verify_trunk_id = atoi(arg[1]);
                if ((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(verify_ret_t, verify_trunk_id);
                }
                else
                {
                    if (FALSE == PPPOE_IA_PMGR_ClearPortStatistics(ifindex))
                    {
                        CLI_LIB_PrintStr_1(
                            "Failed to clear PPPoE intermediate statistics on trunk %lu.\r\n",
                            verify_trunk_id);
                    }
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
            }
        }
#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

    return CLI_NO_ERROR;
}

