/* MODULE NAME: cli_api_pfc.c
 * PURPOSE:
 *   Definitions of CLI APIs for Priority-based Flow Control.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   08/16/12    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2012
 */

 /* INCLUDE FILE DECLARATIONS
  */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "cli_api.h"
#include "cli_lib.h"

#include "cli_api_pfc.h"
#include "swctrl.h"
#include "nmtr_pmgr.h"
#include "pfc_pmgr.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* command: [no] pfc mode {auto | on}
 */
UI32_T CLI_API_PFC_Mode_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PFC == TRUE)
    UI32_T  lport   = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port=0;
    UI32_T  verify_ret=0;
    UI32_T  i = 0;
    UI32_T  pfc_port_mode = PFC_TYPE_PMODE_OFF;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_PFC_MODE:
        break;
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_PFC_MODE:
        break;
    default:
        return CLI_NO_ERROR;
    }

    if (NULL != arg[0])
    {
        switch (arg[0][0])
        {
        case 'a':
        case 'A':
            pfc_port_mode = PFC_TYPE_PMODE_AUTO;
            break;
        case 'o':
        case 'O':
            pfc_port_mode = PFC_TYPE_PMODE_ON;
            break;
        }
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

            if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(
                            lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &pfc_port_mode))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2(
                    "Failed to configure PFC mode on port %lu/%lu.\r\n",
                    (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
            }
        }
    }
#endif /* #if (SYS_CPNT_PFC == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_PFC_Mode_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PFC == TRUE)
    UI32_T  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T  lport;
    UI32_T  pfc_port_mode = PFC_TYPE_PMODE_OFF;
    UI32_T  verify_ret;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_PFC_MODE:
        break;
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_PFC_MODE:
        break;
    default:
        return CLI_NO_ERROR;
    }

    if (NULL != arg[0])
    {
        switch (arg[0][0])
        {
        case 'a':
        case 'A':
            pfc_port_mode = PFC_TYPE_PMODE_AUTO;
            break;
        case 'o':
        case 'O':
            pfc_port_mode = PFC_TYPE_PMODE_ON;
            break;
        }
    }

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
    }
    else
    {
        if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(
                        lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &pfc_port_mode))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1(
                "Failed to configure PFC mode on trunk %lu.\r\n",
                (unsigned long)verify_trunk_id);
#endif
        }
    }
#endif /* #if (SYS_CPNT_PFC == TRUE) */

    return CLI_NO_ERROR;
}

/* command: [no] pfc priority enable pri_lst
 */
UI32_T CLI_API_PFC_PriorityEnable_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PFC == TRUE)
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  lport =0, verify_port =0, verify_ret =0;
    UI32_T  i = 0, pfc_pri_bmp = 0, lower_val, upper_val, err_idx, pri, fld_id;
    char    *tok_p;
    char    delemiters[2] = {","};
    char    tok_buf[CLI_DEF_MAX_BUFSIZE] = {0};

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_PFC_PRIORITY_ENABLE:
        fld_id = PFC_TYPE_FLDE_PORT_PRI_EN_ADM_ADD;

        if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
        {
            /* need to overwrite the default setting, not just add to it
             */
            fld_id = PFC_TYPE_FLDE_PORT_PRI_EN_ADM;
        }
        break;
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_PFC_PRIORITY_ENABLE:
        fld_id = PFC_TYPE_FLDE_PORT_PRI_EN_ADM_REM;
        break;
    default:
        return CLI_NO_ERROR;
    }

    tok_p = arg[0];
    if (NULL == tok_p)
    {
        return CLI_ERR_INTERNAL;
    }

    while (NULL != tok_p)
    {
        tok_p = CLI_LIB_Get_Token(tok_p, tok_buf, delemiters);
        if (FALSE == CLI_LIB_Get_Lower_Upper_Value(tok_buf, &lower_val, &upper_val, &err_idx))
            break;

        for (pri = lower_val; pri <= upper_val; pri++)
        {
            pfc_pri_bmp |= (0x1 << pri);
        }
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

            if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(
                            lport, fld_id, &pfc_pri_bmp))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_3(
                    "Failed to configure PFC priority %s on port %ld/%ld.\r\n",
                    arg[0], (long)verify_unit, (long)verify_port);
#endif
            }
        }
    }
#endif /* #if (SYS_CPNT_PFC == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_PFC_PriorityEnable_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PFC == TRUE)
    UI32_T  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T  lport =0, verify_ret =0;
    UI32_T  pfc_pri_bmp = 0, lower_val, upper_val, err_idx, pri, fld_id;
    char    *tok_p;
    char    delemiters[2] = {","};
    char    tok_buf[CLI_DEF_MAX_BUFSIZE] = {0};

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_PFC_PRIORITY_ENABLE:
        fld_id = PFC_TYPE_FLDE_PORT_PRI_EN_ADM_ADD;
        break;
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_PFC_PRIORITY_ENABLE:
        fld_id = PFC_TYPE_FLDE_PORT_PRI_EN_ADM_REM;
        break;
    default:
        return CLI_NO_ERROR;
    }

    tok_p = arg[0];
    if (NULL == tok_p)
    {
        return CLI_ERR_INTERNAL;
    }

    while (NULL != tok_p)
    {
        tok_p = CLI_LIB_Get_Token(tok_p, tok_buf, delemiters);
        if (FALSE == CLI_LIB_Get_Lower_Upper_Value(tok_buf, &lower_val, &upper_val, &err_idx))
            break;

        for (pri = lower_val; pri <= upper_val; pri++)
        {
            pfc_pri_bmp |= (0x1 << pri);
        }
    }

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
    }
    else
    {
        if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(
                        lport, fld_id, &pfc_pri_bmp))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_2(
                "Failed to configure PFC priority %s on trunk %ld.\r\n",
                arg[0], (long)verify_trunk_id);
#endif
        }
    }
#endif /* #if (SYS_CPNT_PFC == TRUE) */

    return CLI_NO_ERROR;
}

/* command: [no] pfc link-delay-allowance lda_bits
 */
UI32_T CLI_API_PFC_LinkDelayAllowance_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0 //not supported yet (SYS_CPNT_PFC == TRUE)
    UI32_T  lport =0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port =0;
    UI32_T  verify_ret =0;
    UI32_T  i =0;
    UI32_T  pfc_lda_bits = PFC_TYPE_DFT_LDA_BITS;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_PFC_LINKDELAYALLOWANCE:
        break;
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_PFC_LINKDELAYALLOWANCE:
        break;
    default:
        return CLI_NO_ERROR;
    }

    if (NULL != arg[0])
    {
        pfc_lda_bits = atoi(arg[0]);
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

            if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(
                            lport, PFC_TYPE_FLDE_PORT_LDA, &pfc_lda_bits))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2(
                    "Failed to configure PFC link delay allownance on port %lu/%lu.\r\n",
                    verify_unit, verify_port);
#endif
            }
        }
    }
#endif /* #if (SYS_CPNT_PFC == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_PFC_LinkDelayAllowance_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0 //not supported yet (SYS_CPNT_PFC == TRUE)
    UI32_T  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T  lport =0;
    UI32_T  verify_ret =0;
    UI32_T  pfc_lda_bits = PFC_TYPE_DFT_LDA_BITS;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_PFC_LINKDELAYALLOWANCE:
        break;
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_PFC_LINKDELAYALLOWANCE:
        break;
    default:
        return CLI_NO_ERROR;
    }

    if (NULL != arg[0])
    {
        pfc_lda_bits = atoi(arg[0]);
    }

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
    }
    else
    {
        if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(
                        lport, PFC_TYPE_FLDE_PORT_LDA, &pfc_lda_bits))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1(
                "Failed to configure PFC link delay allownance on trunk %lu.\r\n",
                verify_trunk_id);
#endif
        }
    }
#endif /* #if (SYS_CPNT_PFC == TRUE) */

    return CLI_NO_ERROR;
}

/* command: clear pfc statistics [interface eth]
 */
UI32_T CLI_API_PFC_ClearStatistics(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PFC == TRUE)
        UI32_T              ifindex;
        UI32_T              verify_unit;
        UI32_T              verify_port;
        UI32_T              current_max_unit = 0;
        UI32_T              verify_trunk_id;
        CLI_API_EthStatus_T     verify_ret_e;
        CLI_API_TrunkStatus_T   verify_ret_t;

#if (SYS_CPNT_STACKING == TRUE)
        current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
        current_max_unit = 1;
#endif

        if (arg[0] == NULL)
        {
            for (ifindex =0; ifindex < SYS_ADPT_TOTAL_NBR_OF_LPORT; ifindex++)
            {
                NMTR_PMGR_ClearSystemwidePfcStats(ifindex);
            }
#if 0
            if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(
                            0, PFC_TYPE_FLDE_PORT_CLR_STATIS, &tmp_val))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr(
                    "Failed to clear PFC statistics of all ports.\r\n");
#endif
            }
#endif
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
                    NMTR_PMGR_ClearSystemwidePfcStats(ifindex);
#if 0
                    if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(
                            ifindex, PFC_TYPE_FLDE_PORT_CLR_STATIS, &tmp_val))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2(
                            "Failed to clear PFC statistics on port %lu/%lu.\r\n",
                            verify_unit, verify_port);
#endif
                    }
#endif
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
                    NMTR_PMGR_ClearSystemwidePfcStats(ifindex);

#if 0
                    if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(
                            ifindex, PFC_TYPE_FLDE_PORT_CLR_STATIS, &tmp_val))
                    {
                        CLI_LIB_PrintStr_1(
                            "Failed to clear PFC statistics on trunk %lu.\r\n",
                            verify_trunk_id);
                    }
#endif
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
            }
        }
#endif /* #if (SYS_CPNT_PFC == TRUE) */

    return CLI_NO_ERROR;
}


#if (SYS_CPNT_PFC == TRUE)

typedef     UI32_T (*TITLE_FUN_P) (UI32_T, char *);
typedef     UI32_T (*OLINE_FUN_P) (UI32_T, UI32_T, char *);


static UI32_T Show_PFC_Info_Title(
    UI32_T  line_num,
    char    *buff)
{
    sprintf(buff, "%-9s %-8s %-8s %-15s %-15s\r\n",
                  "Interface", "Admin", "Oper",
                  "Admin", "Oper");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "%-9s %-8s %-8s %-15s %-15s\r\n",
                  "", "Mode", "Mode", "Enabled Pri",
                  "Enabled Pri");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "%-9s %-8s %-8s %-15s %-15s\r\n",
                  "---------", "--------", "--------",
                  "---------------", "---------------");
    PROCESS_MORE_FUNC(buff);
    return line_num;
}

/* Sample:
 * Interface Admin    Oper     Admin           Oper
 *           Mode     Mode     Enabled Pri     Enabled Pri
 * --------- -------- -------- --------------- ---------------
 *         9        8        8              15              15
 * Eth 1/1   Auto     Enabled  0,1,2,3,4,5,6,7 0,1,2,3,4,5,6,7
 * Eth 1/2   Auto     Disabled
 * Eth 1/3   Enabled  Enabled
 * Eth 1/4   Disabled Disabled
 */
#define _PFC_REVERSE_BMP_BYTE(a)            \
            ({                              \
                UI8_T   v = a;              \
                UI8_T   r = v;              \
                int     s = 7;              \
                                            \
                for (v >>= 1; v; v >>= 1)   \
                {                           \
                  r <<= 1;                  \
                  r |= v & 1;               \
                  s--;                      \
                }                           \
                r <<= s;                    \
                r;                          \
            })

static UI32_T Show_PFC_Info_One(
    UI32_T  lport,
    UI32_T  line_num,
    char    *buff)
{
    UI32_T                      unit, port, trunk_id;
    UI32_T                      adm_tag_id = PFC_TYPE_PMODE_MAX,
                                opr_tag_id = PFC_TYPE_PMODE_MAX,
                                tmp_val;
    SWCTRL_Lport_Type_T         port_type;
    char                        inf_str[20] = {0};
    static char                 *mode_tag[] = {
                                        [PFC_TYPE_PMODE_AUTO] = "Auto",
                                        [PFC_TYPE_PMODE_ON  ] = "Enabled",
                                        [PFC_TYPE_PMODE_OFF ] = "Disabled",
                                        [PFC_TYPE_PMODE_MAX ] = "Error" };
    char                        adm_pri_buf[16] = {"Error"},
                                opr_pri_buf[16] = {"Error"};

    port_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    if (  (port_type == SWCTRL_LPORT_TRUNK_PORT)
        ||(port_type == SWCTRL_LPORT_NORMAL_PORT)
       )
    {
        if (TRUE != PFC_PMGR_GetDataByField(
                        lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &adm_tag_id))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            if (port_type == SWCTRL_LPORT_NORMAL_PORT)
            {
                sprintf(buff, "Failed to display PFC information on port %lu/%lu.\r\n",
                    (unsigned long)unit, (unsigned long)port);
            }
            else
            {
                sprintf(buff, "Failed to display PFC information on trunk %lu.\r\n",
                    (unsigned long)trunk_id);
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
        sprintf(inf_str, "Eth %lu/%2lu", (unsigned long)unit, (unsigned long)port);
    }
    else
    {
        sprintf(inf_str, "Trunk %2lu", (unsigned long)trunk_id);
    }

    PFC_PMGR_GetDataByField(
                        lport, PFC_TYPE_FLDE_PORT_MODE_OPR, &opr_tag_id);

    if (TRUE == PFC_PMGR_GetDataByField(
                        lport, PFC_TYPE_FLDE_PORT_PRI_EN_ADM, &tmp_val))
    {
        /* need to reverse the order of bits for CLI_LIB_Bitmap_To_List
         */
        char    tmp_bmp;

        tmp_bmp = _PFC_REVERSE_BMP_BYTE(tmp_val & 0xff);

        CLI_LIB_Bitmap_To_List_Ex(&tmp_bmp,
            adm_pri_buf, sizeof(adm_pri_buf), 8, 0, TRUE);
    }

    if (TRUE == PFC_PMGR_GetDataByField(
                        lport, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &tmp_val))
    {
        /* need to reverse the order of bits for CLI_LIB_Bitmap_To_List
         */
        char    tmp_bmp;

        tmp_bmp = _PFC_REVERSE_BMP_BYTE(tmp_val & 0xff);

        CLI_LIB_Bitmap_To_List_Ex(&tmp_bmp,
            opr_pri_buf, sizeof(opr_pri_buf), 8, 0, TRUE);
    }

    sprintf(buff, "%-9s %-8s %-8s %-15s %-15s\r\n",
            inf_str,
            mode_tag[adm_tag_id],
            mode_tag[opr_tag_id],
            adm_pri_buf,
            opr_pri_buf
            );
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

/* Sample:
 * Interface Pri Rx PFC Frames        Tx PFC Frames
 * --------- --- -------------------- --------------------
 *         9   3                   20                   20
 * Eth 1/1     0                    0                    0
 * Eth 1/2     1                    0                    0
 * Eth 1/3     2                    0                    0
 * Eth 1/4     3                    0                    0
 */
static UI32_T Show_PFC_Statistics_Title(
    UI32_T  line_num,
    char    *buff)
{
    sprintf(buff, "%-9s %-3s %-20s %-20s\r\n",
                  "Interface", "Pri", "Rx PFC Frames", "Tx PFC Frames");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "%-9s %-3s %-20s %-20s\r\n",
                  "---------", "---", "--------------------", "--------------------");
    PROCESS_MORE_FUNC(buff);
    return line_num;
}

#define PFC_UI64_2_STR(value,str)   \
            {                       \
                L_STDLIB_UI64toa (L_STDLIB_UI64_H32(value), L_STDLIB_UI64_L32(value), (char *)str); \
                L_STDLIB_Trim_Left((char *)str, sizeof(str)-1); \
            }

static UI32_T Show_PFC_Statistics_One(
    UI32_T  lport,
    UI32_T  line_num,
    char    *buff)
{
    UI32_T                      unit, port, trunk_id, pri;
    SWCTRL_Lport_Type_T         port_type;
    SWDRV_PfcStats_T            pfc_stats;
    char                        inf_str[20] = {0};
    char                        str1[21] = {0};
    char                        str2[21] = {0};

    port_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    if (  (port_type == SWCTRL_LPORT_TRUNK_PORT)
        ||(port_type == SWCTRL_LPORT_NORMAL_PORT)
       )
    {
        if (TRUE != NMTR_PMGR_GetSystemwidePfcStats(
                        lport, &pfc_stats))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            if (port_type == SWCTRL_LPORT_NORMAL_PORT)
            {
                sprintf(buff, "Failed to display PFC statistics on port %lu/%lu.\r\n",
                    (unsigned long)unit, (unsigned long)port);
            }
            else
            {
                sprintf(buff, "Failed to display PFC statistics on trunk %lu.\r\n",
                    (unsigned long)trunk_id);
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
        sprintf(inf_str, "Eth %lu/%2lu", (unsigned long)unit, (unsigned long)port);
    }
    else
    {
        sprintf(inf_str, "Trunk %2lu", (unsigned long)trunk_id);
    }

    for (pri = 0; pri <=7; pri++)
    {
        PFC_UI64_2_STR(pfc_stats.pri[pri].ieee8021PfcIndications, str1);
        PFC_UI64_2_STR(pfc_stats.pri[pri].ieee8021PfcRequests, str2);

        sprintf(buff, "%-9s %3lu %20s %20s\r\n",
                inf_str,
                (unsigned long)pri,
                str1,
                str2
                );
        PROCESS_MORE_FUNC(buff);
    }

    return line_num;
}

UI32_T CLI_API_PFC_Show_All(
    UI32_T line_num, char *buff, TITLE_FUN_P title_fun_p, OLINE_FUN_P oline_fun_p)
{
    UI32_T  j, i, max_port_num, lport, trunk_id =0;

    line_num = title_fun_p(line_num, buff);

    for (j=0; STKTPLG_POM_GetNextUnit(&j); )
    {
        max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
        /*eth*/
        for (i = 1 ; i <= max_port_num;i++)
        {
           UI32_T               verify_unit = j;
           UI32_T               verify_port = i;
           CLI_API_EthStatus_T  verify_ret;

           if ( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                continue; /*1. not present; 2. trunk member; 3. unknown*/

           line_num = oline_fun_p(lport, line_num, buff);
           if (JUMP_OUT_MORE == line_num)
           {
                return CLI_NO_ERROR;
           }
           else if (EXIT_SESSION_MORE == line_num)
           {
                return CLI_EXIT_SESSION;
           }
        } /* for (i = 1 ; i <= max_port_num;i++) */
    }/* for (j=0; STKTPLG_POM_GetNextUnit(&j); ) */

    /*trunk*/
    while (TRK_PMGR_GetNextTrunkId(&trunk_id))
    {
        UI32_T  trunk_ifindex = 0;

        SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &trunk_ifindex);

        if (TRK_PMGR_GetTrunkMemberCounts(trunk_id) != 0)
        {
            line_num = oline_fun_p(trunk_ifindex, line_num, buff);
            if (JUMP_OUT_MORE == line_num)
            {
                return CLI_NO_ERROR;
            }
            else if (EXIT_SESSION_MORE == line_num)
            {
                return CLI_EXIT_SESSION;
            }
        }
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_PFC_Show_X(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P,
    TITLE_FUN_P title_fun_p, OLINE_FUN_P oline_fun_p)
{
#if (SYS_CPNT_PFC == TRUE)
    UI32_T  ifindex;
    UI32_T  line_num = 0;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    UI32_T  verify_trunk_id = 0;
    CLI_API_EthStatus_T     verify_ret_e;
    CLI_API_TrunkStatus_T   verify_ret_t;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    if (arg[0] == NULL)
    {
        return CLI_API_PFC_Show_All(line_num, buff, title_fun_p, oline_fun_p);
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
                line_num = title_fun_p(line_num, buff);
                oline_fun_p(ifindex, line_num, buff);
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
                line_num = title_fun_p(line_num, buff);
                oline_fun_p(ifindex, line_num, buff);
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
        }
    }
#endif /* #if (SYS_CPNT_PFC == TRUE) */

    return CLI_NO_ERROR;
}
#endif /* #if (SYS_CPNT_PFC == TRUE) */

/* command: show pfc [statistics] [interface eth]
 */
UI32_T CLI_API_PFC_Show(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PFC == TRUE)

    if (NULL == arg[0])
    {
        return CLI_API_PFC_Show_X(cmd_idx, arg, ctrl_P,
            Show_PFC_Info_Title, Show_PFC_Info_One);
    }
    else
    {
        switch (arg[0][0])
        {
        case 's':
        case 'S':
            return CLI_API_PFC_Show_X(cmd_idx, &arg[1], ctrl_P,
                Show_PFC_Statistics_Title, Show_PFC_Statistics_One);

        case 'i':
        case 'I':
            return CLI_API_PFC_Show_X(cmd_idx, arg, ctrl_P,
                Show_PFC_Info_Title, Show_PFC_Info_One);

        default:
            return CLI_ERR_INTERNAL;
        }
    }
#endif /* #if (SYS_CPNT_PFC == TRUE) */

    return CLI_NO_ERROR;
}

