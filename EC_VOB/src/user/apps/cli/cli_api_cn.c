/* MODULE NAME - CLI_API_CN.C
 * PURPOSE : Provides the definitions for CN CLI APIs.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <stdlib.h>
#include <string.h>
#include "sys_type.h"
#include "sys_cpnt.h"
#include "cli_api.h"
#include "cli_api_cn.h"
#include "cli_cmd.h"
#include "cli_msg.h"
#if (SYS_CPNT_CN == TRUE)
#include "cn_pmgr.h"
#include "cn_pom.h"
#include "cn_type.h"
#include "swctrl_pom.h"
#include "swctrl_pmgr.h"
#include "nmtr_pmgr.h"
#include "swdrv_type.h"
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

/* FUNCTION NAME - CLI_API_CN
 * PURPOSE : Set CN global status in global configuration mode.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_CN(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CN == TRUE)
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  status;
    UI32_T  result;

    /* BODY
     */

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W1_CN:
        status = CN_TYPE_GLOBAL_STATUS_ENABLE;
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_CN:
        status = CN_TYPE_GLOBAL_STATUS_DISABLE;
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

    result = CN_PMGR_SetGlobalAdminStatus(status);
    if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr_N("Warning: number of CPs reaches maximum on some ports.\r\n");
    #endif
    }
    else if (result != CN_TYPE_RETURN_OK)
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr_N("Failed to %s congestion notification globally.\r\n",
            (status == CN_TYPE_GLOBAL_STATUS_ENABLE) ? "enable" : "disable");
    #endif
    }
#endif /* end #if (SYS_CPNT_CN == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_CN */

/* FUNCTION NAME - CLI_API_CN_CnmTransmitPriority
 * PURPOSE : Set the priority used for CNM transmission in global configuration
 *           mode.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_CN_CnmTransmitPriority(UI16_T cmd_idx, char *arg[],
                                      CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CN == TRUE)
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  priority;

    /* BODY
     */

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_CN_CNMTRANSMITPRIORITY:
        priority = (UI32_T)atoi(arg[0]);
        if (CN_POM_IsCnpv(priority) == TRUE)
        {
            CLI_LIB_PrintStr_N("Warning: the priority %lu is a CNPV.\r\n",
                (unsigned long)priority);
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_CN_CNMTRANSMITPRIORITY:
        priority = SYS_DFLT_CN_CNM_TRANSMIT_PRIORITY;
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

    if (CN_PMGR_SetCnmTxPriority(priority) != CN_TYPE_RETURN_OK)
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr_N("Failed to set the priority used for CNM transmission.\r\n");
    #endif
    }
#endif /* end #if (SYS_CPNT_CN == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_CN_CnmTransmitPriority */

/* FUNCTION NAME - CLI_API_CN_Cnpv
 * PURPOSE : Set per-CNPV parameters in global configuration mode.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_CN_Cnpv(UI16_T cmd_idx, char *arg[],
                       CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CN == TRUE)
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  priority;
    UI32_T  mode;
    UI32_T  alt_priority;
    UI32_T  result;

    /* BODY
     */

    priority = (UI32_T)atoi(arg[0]);
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_CN_CNPV:
        if (arg[1] == NULL)
        {
            result = CN_PMGR_SetCnpv(priority, TRUE);
            if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_N("Warning: number of CPs reaches maximum on "
                    "some ports.\r\n");
    #endif
            }
            else if (result == CN_TYPE_RETURN_ERROR_CNPV_CAPACITY)
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_N("Failed because the number of CNPVs reaches "
                    "maximum.\r\n");
    #endif
            }
            else if (result != CN_TYPE_RETURN_OK)
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_N("Failed to set priority %lu to be a CNPV.\r\n", (unsigned long)priority);
    #endif
            }
            return CLI_NO_ERROR;
        }

        if (CN_POM_IsCnpv(priority) == FALSE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr_N("The priority %lu is not a CNPV.\r\n", (unsigned long)priority);
    #endif
            return CLI_NO_ERROR;
        }

        if ((arg[1][0] == 'd') || (arg[1][0] == 'D'))
        {
            switch (arg[2][0])
            {
            case 'a':
            case 'A':
                mode = CN_TYPE_DEFENSE_MODE_AUTO;
                break;

            case 'd':
            case 'D':
                mode = CN_TYPE_DEFENSE_MODE_DISABLED;
                break;

            case 'e':
            case 'E':
                mode = CN_TYPE_DEFENSE_MODE_EDGE;
                break;

            case 'i':
            case 'I':
                if (arg[2][8] == '\0')
                {
                    mode = CN_TYPE_DEFENSE_MODE_INTERIOR;
                }
                else
                {
                    mode = CN_TYPE_DEFENSE_MODE_INTERIOR_READY;
                }
                break;

            default:
               return CLI_ERR_INTERNAL;
            }

            result = CN_PMGR_SetCnpvDefenseMode(priority, mode);
            if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_N("Warning: number of CPs reaches maximum on "
                    "some ports.\r\n");
    #endif
            }
            else if (result != CN_TYPE_RETURN_OK)
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_N("Failed to set defense mode for CNPV %lu.\r\n", (unsigned long)priority);
    #endif
            }
        }
        else if ((arg[1][0] == 'a') || (arg[1][0] == 'A'))
        {
            alt_priority = (UI32_T)atoi(arg[2]);
            if (CN_POM_IsCnpv(alt_priority) == TRUE)
            {
                CLI_LIB_PrintStr_N("Warning: the priority %lu is a CNPV.\r\n", (unsigned long)alt_priority);
            }
            if (CN_PMGR_SetCnpvAlternatePriority(priority, alt_priority) != CN_TYPE_RETURN_OK)
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_N("Failed to set alternate priority for CNPV %lu.\r\n", (unsigned long)priority);
    #endif
            }
        }
        else
        {
            return CLI_ERR_INTERNAL;
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_CN_CNPV:
        if (arg[1] == NULL)
        {
            result = CN_PMGR_SetCnpv(priority, FALSE);
            if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_N("Warning: number of CPs reaches maximum on "
                    "some ports.\r\n");
    #endif
            }
            else if (result != CN_TYPE_RETURN_OK)
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_N("Failed to set priority %lu to be a non-CNPV.\r\n", (unsigned long)priority);
    #endif
            }
            return CLI_NO_ERROR;
        }

        if (CN_POM_IsCnpv(priority) == FALSE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr_N("The priority %lu is not a CNPV.\r\n", (unsigned long)priority);
    #endif
            return CLI_NO_ERROR;
        }

        if ((arg[1][0] == 'd') || (arg[1][0] == 'D'))
        {
            mode = SYS_DFLT_CN_CNPV_DEFENSE_MODE;
            result = CN_PMGR_SetCnpvDefenseMode(priority, mode);
            if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_N("Warning: number of CPs reaches maximum on "
                    "some ports.\r\n");
    #endif
            }
            else if (result != CN_TYPE_RETURN_OK)
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_N("Failed to set defense mode for CNPV %lu.\r\n", (unsigned long)priority);
    #endif
            }
        }
        else if ((arg[1][0] == 'a') || (arg[1][0] == 'A'))
        {
            alt_priority = SYS_DFLT_CN_CNPV_ALTERNATE_PRIORITY;
            if (CN_POM_IsCnpv(alt_priority) == TRUE)
            {
                CLI_LIB_PrintStr_N("Warning: the priority %lu is a CNPV.\r\n", (unsigned long)alt_priority);
            }
            if (CN_PMGR_SetCnpvAlternatePriority(priority, alt_priority) != CN_TYPE_RETURN_OK)
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_N("Failed to set alternate priority for CNPV %lu.\r\n", (unsigned long)priority);
    #endif
            }
        }
        else
        {
            return CLI_ERR_INTERNAL;
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif /* end #if (SYS_CPNT_CN == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_CN_Cnpv */

/* FUNCTION NAME - CLI_API_CN_Cnpv_Eth
 * PURPOSE : Set per-CNPV parameters in ethernet interface configuration mode.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_CN_Cnpv_Eth(UI16_T cmd_idx, char *arg[],
                           CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CN == TRUE)
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  priority;
    BOOL_T  set_mode;
    BOOL_T  set_alt_priority;
    UI32_T  mode = 0;
    UI32_T  alt_priority = 0;
    UI32_T  i, unit, port, lport;
    UI32_T  result;

    /* BODY
     */

    if (arg[1] == NULL)
    {
        return CLI_ERR_INTERNAL;
    }

    priority = (UI32_T)atoi(arg[0]);
    if (CN_POM_IsCnpv(priority) == FALSE)
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr_N("The priority %lu is not a CNPV.\r\n", (unsigned long)priority);
    #endif
        return CLI_NO_ERROR;
    }

    set_mode = set_alt_priority = FALSE;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_CN_CNPV:
        if ((arg[1][0] == 'd') || (arg[1][0] == 'D'))
        {
            switch (arg[2][0])
            {
            case 'a':
            case 'A':
                mode = CN_TYPE_DEFENSE_MODE_AUTO;
                break;

            case 'd':
            case 'D':
                mode = CN_TYPE_DEFENSE_MODE_DISABLED;
                break;

            case 'e':
            case 'E':
                mode = CN_TYPE_DEFENSE_MODE_EDGE;
                break;

            case 'i':
            case 'I':
                if (arg[2][8] == '\0')
                {
                    mode = CN_TYPE_DEFENSE_MODE_INTERIOR;
                }
                else
                {
                    mode = CN_TYPE_DEFENSE_MODE_INTERIOR_READY;
                }
                break;

            default:
               return CLI_ERR_INTERNAL;
            }

            set_mode = TRUE;
        }
        else if ((arg[1][0] == 'a') || (arg[1][0] == 'A'))
        {
            alt_priority = (UI32_T)atoi(arg[2]);
            if (CN_POM_IsCnpv(alt_priority) == TRUE)
            {
                CLI_LIB_PrintStr_N("Warning: the priority %lu is a CNPV.\r\n", (unsigned long)alt_priority);
            }
            set_alt_priority = TRUE;
        }
        else
        {
            return CLI_ERR_INTERNAL;
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_CN_CNPV:
        if ((arg[1][0] == 'd') || (arg[1][0] == 'D'))
        {
            mode = CN_TYPE_DEFENSE_MODE_BY_GLOBAL;
            set_mode = TRUE;
        }
        else if ((arg[1][0] == 'a') || (arg[1][0] == 'A'))
        {
            alt_priority = CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL;
            if (CN_POM_IsCnpv(alt_priority) == TRUE)
            {
                CLI_LIB_PrintStr_N("Warning: the priority %lu is a CNPV.\r\n", (unsigned long)alt_priority);
            }
            set_alt_priority = TRUE;
        }
        else
        {
            return CLI_ERR_INTERNAL;
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

    unit = ctrl_P->CMenu.unit_id;
    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)] & (1 << ( 7 - ((i-1)%8))))
        {
            port = i;

            if ((result = verify_ethernet(unit, port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(result, unit, port);
                continue;
            }

            if (set_mode == TRUE)
            {
                result = CN_PMGR_SetPortCnpvDefenseMode(priority, lport, mode);
                if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr_N("Warning: number of CPs reaches maximum "
                        "on ethernet port %lu/%lu.\r\n", (unsigned long)unit, (unsigned long)port);
    #endif
                }
                else if (result != CN_TYPE_RETURN_OK)
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr_N("Failed to set defense mode for CNPV %lu"
                        " on ethernet port %lu/%lu.\r\n", (unsigned long)priority, (unsigned long)unit, (unsigned long)port);
    #endif
                }
            }
            else if (set_alt_priority == TRUE)
            {
                if (CN_PMGR_SetPortCnpvAlternatePriority(priority, lport, alt_priority) != CN_TYPE_RETURN_OK)
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr_N("Failed to set alternate priority for "
                        "CNPV %lu on ethernet port %lu/%lu.\r\n", (unsigned long)priority, (unsigned long)unit, (unsigned long)port);
    #endif
                }
            }
        }
    }
#endif /* end #if (SYS_CPNT_CN == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_CN_Cnpv_Eth */

/* FUNCTION NAME - CLI_API_CN_Cnpv_Pch
 * PURPOSE : Set per-CNPV parameters in port channel interface configuration
 *           mode.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_CN_Cnpv_Pch(UI16_T cmd_idx, char *arg[],
                           CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
#if (SYS_CPNT_CN == TRUE)
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  priority;
    BOOL_T  set_mode;
    BOOL_T  set_alt_priority;
    UI32_T  mode = 0;
    UI32_T  alt_priority = 0;
    UI32_T  trunk_id, lport;
    UI32_T  result;

    /* BODY
     */

    if (arg[1] == NULL)
    {
        return CLI_ERR_INTERNAL;
    }

    priority = (UI32_T)atoi(arg[0]);
    if (CN_POM_IsCnpv(priority) == FALSE)
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr_N("The priority %lu is not a CNPV.\r\n", (unsigned long)priority);
    #endif
        return CLI_NO_ERROR;
    }

    set_mode = set_alt_priority = FALSE;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_CN_CNPV:
        if ((arg[1][0] == 'd') || (arg[1][0] == 'D'))
        {
            switch (arg[2][0])
            {
            case 'a':
            case 'A':
                mode = CN_TYPE_DEFENSE_MODE_AUTO;
                break;

            case 'd':
            case 'D':
                mode = CN_TYPE_DEFENSE_MODE_DISABLED;
                break;

            case 'e':
            case 'E':
                mode = CN_TYPE_DEFENSE_MODE_EDGE;
                break;

            case 'i':
            case 'I':
                if (arg[2][8] == '\0')
                {
                    mode = CN_TYPE_DEFENSE_MODE_INTERIOR;
                }
                else
                {
                    mode = CN_TYPE_DEFENSE_MODE_INTERIOR_READY;
                }
                break;

            default:
               return CLI_ERR_INTERNAL;
            }

            set_mode = TRUE;
        }
        else if ((arg[1][0] == 'a') || (arg[1][0] == 'A'))
        {
            alt_priority = (UI32_T)atoi(arg[2]);
            if (CN_POM_IsCnpv(alt_priority) == TRUE)
            {
                CLI_LIB_PrintStr_N("Warning: the priority %lu is a CNPV.\r\n", (unsigned long)alt_priority);
            }
            set_alt_priority = TRUE;
        }
        else
        {
            return CLI_ERR_INTERNAL;
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_CN_CNPV:
        if ((arg[1][0] == 'd') || (arg[1][0] == 'D'))
        {
            mode = CN_TYPE_DEFENSE_MODE_BY_GLOBAL;
            set_mode = TRUE;
        }
        else if ((arg[1][0] == 'a') || (arg[1][0] == 'A'))
        {
            alt_priority = CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL;
            if (CN_POM_IsCnpv(alt_priority) == TRUE)
            {
                CLI_LIB_PrintStr_N("Warning: the priority %lu is a CNPV.\r\n", (unsigned long)alt_priority);
            }
            set_alt_priority = TRUE;
        }
        else
        {
            return CLI_ERR_INTERNAL;
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

    trunk_id = ctrl_P->CMenu.pchannel_id;
    if ((result = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(result, trunk_id);
        return CLI_NO_ERROR;
    }

    if (set_mode == TRUE)
    {
        result = CN_PMGR_SetPortCnpvDefenseMode(priority, lport, mode);
        if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr_N("Warning: number of CPs reaches maximum on port "
                "channel %lu.\r\n", (unsigned long)trunk_id);
    #endif
        }
        else if (result != CN_TYPE_RETURN_OK)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr_N("Failed to set defense mode for CNPV %lu on port"
                " channel %lu.\r\n", (unsigned long)priority, (unsigned long)trunk_id);
    #endif
        }
    }
    else if (set_alt_priority == TRUE)
    {
        if (CN_PMGR_SetPortCnpvAlternatePriority(priority, lport, alt_priority) != CN_TYPE_RETURN_OK)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr_N("Failed to set alternate priority for CNPV %lu "
                "on port channel %lu.\r\n", (unsigned long)priority, (unsigned long)trunk_id);
    #endif
        }
    }
#endif /* end #if (SYS_CPNT_CN == TRUE) */
#endif /* #if 0 */
    return CLI_NO_ERROR;
} /* End of CLI_API_CN_Cnpv_Pch */

/* FUNCTION NAME - CLI_API_CN_ShowCn
 * PURPOSE : Show CN global information in privilege execution mode
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES   : None
 */
UI32_T CLI_API_CN_ShowCn(UI16_T cmd_idx, char *arg[],
                         CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CN == TRUE)
    UI32_T                  line_num = 0;
    char                    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    CN_TYPE_GlobalEntry_T   global_entry;
    int                     left_width;
    UI64_T                  count;
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
    CN_TYPE_CpEntry_T   	cp_entry;
    SWDRV_IfPerQStats_T     if_perq_stats;
    UI32_T                  ifindex, i;
#endif

    memset(&global_entry, 0, sizeof(CN_TYPE_GlobalEntry_T));
    if (CN_POM_GetGlobalEntry(&global_entry) != CN_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr_N("Failed to get the congestion notification global information.\r\n");
        return CLI_NO_ERROR;
    }

    count = 0;
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
    ifindex = 0;
    while (NMTR_PMGR_GetNextSystemwideIfPerQStats(&ifindex, &if_perq_stats) == TRUE)
    {
        for (i = 0; i < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; i++)
        {
            cp_entry.lport = ifindex;
            cp_entry.cp_index = i;
            if (CN_POM_GetCpEntry(&cp_entry) != CN_TYPE_RETURN_OK)
            {
                continue;
            }
            count += if_perq_stats.cosq[cp_entry.queue].ifOutDiscardPkts;
        }
    }
#endif

    CLI_LIB_PrintStr_N("Congestion Notification Global Information");
    PROCESS_MORE("\r\n");

    left_width = strlen("Total Discarded Frames");

    CLI_LIB_PrintStr_N(" %-*s : %s", left_width, "Admin Status",
        (global_entry.admin_status == CN_TYPE_GLOBAL_STATUS_ENABLE) ? "Enabled" : "Disabled");
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %s", left_width, "Oper Status",
        (global_entry.oper_status == CN_TYPE_GLOBAL_STATUS_ENABLE) ? "Enabled" : "Disabled");
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %lu", left_width, "CNM Transmit Priority", (unsigned long)global_entry.cnm_tx_priority);
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %llu", left_width, "Total Discarded Frames", (unsigned long long)count);
    PROCESS_MORE("\r\n");

    PROCESS_MORE("\r\n");
#endif /* #if (SYS_CPNT_CN == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_CN_ShowCn */

/* FUNCTION NAME - CLI_API_CN_ShowCnpv
 * PURPOSE : Show per-CNPV information in privilege execution mode
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES   : None
 */
UI32_T CLI_API_CN_ShowCnpv(UI16_T cmd_idx, char *arg[],
                           CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CN == TRUE)
    UI32_T                  line_num = 0;
    char                    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T                  priority;
    CN_TYPE_CnpvEntry_T     cnpv_entry;
    CN_TYPE_PortCnpvEntry_T port_cnpv_entry;
    SWCTRL_Lport_Type_T     port_type;
    UI32_T                  unit, port, trunk_id, lport;
    UI32_T                  result;
    int                     left_width;
    char                    *defense_mode_str[] = {
                                [CN_TYPE_DEFENSE_MODE_DISABLED]         = "Disabled",
                                [CN_TYPE_DEFENSE_MODE_EDGE]             = "Edge",
                                [CN_TYPE_DEFENSE_MODE_INTERIOR]         = "Interior",
                                [CN_TYPE_DEFENSE_MODE_INTERIOR_READY]   = "Interior-Ready",
                                [CN_TYPE_DEFENSE_MODE_AUTO]             = "Auto",
                                [CN_TYPE_DEFENSE_MODE_BY_GLOBAL]        = "By-Global"};

    if (arg[0] == NULL)
    {
        CLI_LIB_PrintStr_N("Congestion Notification CNPV Informations");
        PROCESS_MORE("\r\n");
        CLI_LIB_PrintStr_N(" CNPV  DefMode         AdminAltPri  AutoAltPri");
        PROCESS_MORE("\r\n");
        CLI_LIB_PrintStr_N(" ----  --------------  -----------  ----------");
        PROCESS_MORE("\r\n");

        for (priority = CN_TYPE_MIN_PRIORITY; priority <= CN_TYPE_MAX_PRIORITY; priority++)
        {
            if (CN_POM_IsCnpv(priority) == FALSE)
            {
                continue;
            }

            memset(&cnpv_entry, 0, sizeof(CN_TYPE_CnpvEntry_T));
            cnpv_entry.cnpv = priority;
            if (CN_POM_GetCnpvEntry(&cnpv_entry) != CN_TYPE_RETURN_OK)
            {
                CLI_LIB_PrintStr_N("Failed to get the information of CNPV %lu."
                    "\r\n", (unsigned long)priority);
                continue;
            }

            CLI_LIB_PrintStr_N(" %-4lu  %-14s  %-11lu  %-10lu", (unsigned long)cnpv_entry.cnpv,
                defense_mode_str[cnpv_entry.defense_mode],
                (unsigned long)cnpv_entry.admin_alt_priority, (unsigned long)cnpv_entry.auto_alt_priority);
            PROCESS_MORE("\r\n");
        }

        PROCESS_MORE("\r\n");
        return CLI_NO_ERROR;
    }

    priority = (UI32_T)atoi(arg[0]);
    if (CN_POM_IsCnpv(priority) == FALSE)
    {
        CLI_LIB_PrintStr_N("The priority %lu is not a CNPV.\r\n", (unsigned long)priority);
        return CLI_NO_ERROR;
    }

    if (arg[1] == NULL)
    {
        memset(&cnpv_entry, 0, sizeof(CN_TYPE_CnpvEntry_T));
        cnpv_entry.cnpv = priority;
        if (CN_POM_GetCnpvEntry(&cnpv_entry) != CN_TYPE_RETURN_OK)
        {
            CLI_LIB_PrintStr_N("Failed to get the information of CNPV %lu.\r\n", (unsigned long)priority);
            return CLI_NO_ERROR;
        }

        CLI_LIB_PrintStr_N("Congestion Notification Per-CNPV Information");
        PROCESS_MORE("\r\n");

        left_width = strlen("Admin Alternate Priority");

        CLI_LIB_PrintStr_N(" %-*s : %lu", left_width, "CNPV", (unsigned long)cnpv_entry.cnpv);
        PROCESS_MORE("\r\n");

        CLI_LIB_PrintStr_N(" %-*s : %s", left_width, "Defense Mode",
            defense_mode_str[cnpv_entry.defense_mode]);
        PROCESS_MORE("\r\n");

        CLI_LIB_PrintStr_N(" %-*s : %lu", left_width, "Admin Alternate Priority",
            (unsigned long)cnpv_entry.admin_alt_priority);
        PROCESS_MORE("\r\n");

        CLI_LIB_PrintStr_N(" %-*s : %lu", left_width, "Auto Alternate Priority",
            (unsigned long)cnpv_entry.auto_alt_priority);
        PROCESS_MORE("\r\n");

        PROCESS_MORE("\r\n");

        CLI_LIB_PrintStr_N("Congestion Notification Per-CNPV Port Informations");
        PROCESS_MORE("\r\n");

        CLI_LIB_PrintStr_N(" Port      AdminDefMode    OperDefMode     AdminAltPri  OperAltPri");
        PROCESS_MORE("\r\n");
        CLI_LIB_PrintStr_N(" --------  --------------  --------------  -----------  ----------");
        PROCESS_MORE("\r\n");

        lport = 0;
        while ((port_type = SWCTRL_POM_GetNextLogicalPort(&lport)) == SWCTRL_LPORT_NORMAL_PORT)
        {
            port_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
            if ((port_type == SWCTRL_LPORT_UNKNOWN_PORT) ||
                (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER))
            {
                continue;
            }

            memset(&port_cnpv_entry, 0, sizeof(CN_TYPE_PortCnpvEntry_T));
            port_cnpv_entry.cnpv = priority;
            port_cnpv_entry.lport = lport;
            if (CN_POM_GetPortCnpvEntry(&port_cnpv_entry) != CN_TYPE_RETURN_OK)
            {
                if (port_type == SWCTRL_LPORT_NORMAL_PORT)
                {
                    CLI_LIB_PrintStr_N("Failed to get the information of CNPV "
                        "%lu on ethernet port %lu/%lu.\r\n", (unsigned long)priority, (unsigned long)unit, (unsigned long)port);
                }
                else
                {
                    CLI_LIB_PrintStr_N("Failed to get the information of CNPV "
                        "%lu on port channel %lu.\r\n", (unsigned long)priority, (unsigned long)trunk_id);
                }
                return CLI_NO_ERROR;
            }

            if (port_type == SWCTRL_LPORT_NORMAL_PORT)
            {
                CLI_LIB_PrintStr_N(" Eth %lu/%2lu", (unsigned long)unit, (unsigned long)port);
            }
            else
            {
                CLI_LIB_PrintStr_N(" Trunk %2lu", (unsigned long)trunk_id);
            }

            CLI_LIB_PrintStr_N("  %-14s", defense_mode_str[port_cnpv_entry.admin_defense_mode]);
            CLI_LIB_PrintStr_N("  %-14s", defense_mode_str[port_cnpv_entry.oper_defense_mode]);

            if (port_cnpv_entry.admin_alt_priority == CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL)
            {
                CLI_LIB_PrintStr_N("  %-11s", "By-Global");
            }
            else
            {
                CLI_LIB_PrintStr_N("  %-11lu", (unsigned long)port_cnpv_entry.admin_alt_priority);
            }

            CLI_LIB_PrintStr_N("  %-10lu", (unsigned long)port_cnpv_entry.oper_alt_priority);
            PROCESS_MORE("\r\n");
        } /* end of while */

        PROCESS_MORE("\r\n");
        return CLI_NO_ERROR;
    }

    unit = port = trunk_id = 0;
    if ((arg[1][0] == 'e') || (arg[1][0] == 'E'))
    {
        unit = (UI32_T)atoi(arg[2]);
        port = (UI32_T)atoi(strchr(arg[2], '/') + 1);
        if ((result = verify_ethernet(unit, port, &lport)) != CLI_API_ETH_OK)
        {
            display_ethernet_msg(result, unit, port);
            return CLI_NO_ERROR;
        }
    }
    else if ((arg[1][0] == 'p') || (arg[1][0] == 'P'))
    {
        trunk_id = (UI32_T)atoi(arg[2]);
        if ((result = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
        {
            display_trunk_msg(result, trunk_id);
            return CLI_NO_ERROR;
        }
    }
    else
    {
        return CLI_ERR_INTERNAL;
    }

    memset(&port_cnpv_entry, 0, sizeof(CN_TYPE_PortCnpvEntry_T));
    port_cnpv_entry.cnpv = priority;
    port_cnpv_entry.lport = lport;
    if (CN_POM_GetPortCnpvEntry(&port_cnpv_entry) != CN_TYPE_RETURN_OK)
    {
        if (trunk_id == 0)
        {
            CLI_LIB_PrintStr_N("Failed to get the information of CNPV %lu on "
                "ethernet port %lu/%lu.\r\n", (unsigned long)priority, (unsigned long)unit, (unsigned long)port);
        }
        else
        {
            CLI_LIB_PrintStr_N("Failed to get the information of CNPV "
                "%lu on port channel %lu.\r\n", (unsigned long)priority, (unsigned long)trunk_id);
        }

        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr_N("Congestion Notification Per-CNPV Port Information");
    PROCESS_MORE("\r\n");

    left_width = strlen("Admin Alternate Priority");

    CLI_LIB_PrintStr_N(" %-*s : %lu", left_width, "CNPV", (unsigned long)priority);
    PROCESS_MORE("\r\n");

    if (trunk_id == 0)
    {
        CLI_LIB_PrintStr_N(" %-*s : Eth %lu/%lu", left_width, "Port", (unsigned long)unit, (unsigned long)port);
    }
    else
    {
        CLI_LIB_PrintStr_N(" %-*s : Trunk %lu", left_width, "Port", (unsigned long)trunk_id);
    }
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %s", left_width, "Admin Defense Mode",
        defense_mode_str[port_cnpv_entry.admin_defense_mode]);
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %s", left_width, "Oper Defense Mode",
        defense_mode_str[port_cnpv_entry.oper_defense_mode]);
    PROCESS_MORE("\r\n");

    if (port_cnpv_entry.admin_alt_priority == CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL)
    {
        CLI_LIB_PrintStr_N(" %-*s : %s", left_width, "Admin Alternate Priority",
            "By-Global");
    }
    else
    {
        CLI_LIB_PrintStr_N(" %-*s : %lu", left_width, "Admin Alternate Priority",
            (unsigned long)port_cnpv_entry.admin_alt_priority);
    }
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %lu", left_width, "Oper Alternate Priority",
        (unsigned long)port_cnpv_entry.oper_alt_priority);
    PROCESS_MORE("\r\n");

    PROCESS_MORE("\r\n");
#endif /* #if (SYS_CPNT_CN == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_CN_ShowCnpv */

/* FUNCTION NAME - CLI_API_CN_ShowCp
 * PURPOSE : Show per-CP information in privilege execution mode
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES   : None
 */
UI32_T CLI_API_CN_ShowCp(UI16_T cmd_idx, char *arg[],
                         CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CN == TRUE)
    UI32_T              line_num = 0;
    char                buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T              unit, port, trunk_id, lport;
    UI32_T              result;
    UI32_T              cp_index;
    CN_TYPE_CpEntry_T   cp_entry;
    int                 left_width, i, len;
    char                str[CLI_DEF_MAX_BUFSIZE] = {0};
    UI8_T               cpid[CN_TYPE_CPID_SIZE] = {0};
    UI64_T              discarded_frames, transmitted_frames;
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
    SWDRV_IfPerQStats_T if_perq_stats;
#endif
    SWDRV_QcnStats_T    qcn_stats;

    if (arg[0] == NULL)
    {
        return CLI_ERR_INTERNAL;
    }

    unit = port = trunk_id = 0;
    if ((arg[0][0] == 'e') || (arg[0][0] == 'E'))
    {
        unit = (UI32_T)atoi(arg[1]);
        port = (UI32_T)atoi(strchr(arg[1], '/')+1);
        if ((result = verify_ethernet(unit, port, &lport)) != CLI_API_ETH_OK)
        {
            display_ethernet_msg(result, unit, port);
            return CLI_NO_ERROR;
        }
    }
    else if ((arg[0][0] == 'p') || (arg[0][0] == 'P'))
    {
        trunk_id = (UI32_T)atoi(arg[1]);
        if ((result = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
        {
            display_trunk_msg(result, trunk_id);
            return CLI_NO_ERROR;
        }
    }
    else
    {
        return CLI_ERR_INTERNAL;
    }

    if (arg[3] == NULL)
    {
        return CLI_ERR_INTERNAL;
    }

    cp_index = (UI32_T)atoi(arg[3]);

    if (CN_POM_IsCp(lport, cp_index) == FALSE)
    {
        if (trunk_id == 0)
        {
            CLI_LIB_PrintStr_N("There is no CP with index %lu on ethernet "
                "%lu/%lu.\r\n", (unsigned long)cp_index, (unsigned long)unit, (unsigned long)port);
        }
        else
        {
            CLI_LIB_PrintStr_N("There is no CP with index %lu on port channel "
                "%lu.\r\n", (unsigned long)cp_index, (unsigned long)trunk_id);
        }

        return CLI_NO_ERROR;
    }

    memset(&cp_entry, 0, sizeof(CN_TYPE_CpEntry_T));
    cp_entry.lport = lport;
    cp_entry.cp_index = cp_index;
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
    memset(&if_perq_stats, 0, sizeof(SWDRV_IfPerQStats_T));
#endif
    memset(&qcn_stats, 0, sizeof(SWDRV_QcnStats_T));
    result = CN_TYPE_RETURN_OK;
    if (CN_POM_GetCpEntry(&cp_entry) != CN_TYPE_RETURN_OK)
    {
        result = CN_TYPE_RETURN_ERROR;
    }
    else if (SWCTRL_PMGR_GetPortQcnCpid(lport, cp_entry.queue, cpid) == FALSE)
    {
        result = CN_TYPE_RETURN_ERROR;
    }
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
    else if (NMTR_PMGR_GetSystemwideIfPerQStats(lport, &if_perq_stats) == FALSE)
    {
        result = CN_TYPE_RETURN_ERROR;
    }
#endif
    else if (NMTR_PMGR_GetSystemwideQcnStats(lport, &qcn_stats) == FALSE)
    {
        result = CN_TYPE_RETURN_ERROR;
    }

    if (result == CN_TYPE_RETURN_ERROR)
    {
        if (trunk_id == 0)
        {
            CLI_LIB_PrintStr_N("Failed to get the information of CP %lu on "
                "ethernet port %lu/%lu.\r\n", (unsigned long)cp_index, (unsigned long)unit, (unsigned long)port);
        }
        else
        {
            CLI_LIB_PrintStr_N("Failed to get the information of CP %lu on "
                "port channel %lu.\r\n", (unsigned long)cp_index, (unsigned long)trunk_id);
        }

        return CLI_NO_ERROR;
    }

    discarded_frames = transmitted_frames = 0;
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
    discarded_frames = if_perq_stats.cosq[cp_entry.queue].ifOutDiscardPkts;
    transmitted_frames = if_perq_stats.cosq[cp_entry.queue].ifOutPkts;
#endif

    CLI_LIB_PrintStr_N("Congestion Notification Per-Port Per-CP Information");
    PROCESS_MORE("\r\n");

    left_width = strlen("Minimum Sample Base");

    if (trunk_id == 0)
    {
        CLI_LIB_PrintStr_N(" %-*s : Eth %lu/%lu", left_width, "Port", (unsigned long)unit, (unsigned long)port);
    }
    else
    {
        CLI_LIB_PrintStr_N(" %-*s : Trunk %lu", left_width, "Port", (unsigned long)trunk_id);
    }
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %lu", left_width, "CP Index", (unsigned long)cp_index);
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : ", left_width, "CPID");
    for (i = 0; i < CN_TYPE_CPID_SIZE; i++)
    {
        CLI_LIB_PrintStr_N("%02X", cpid[i]);
    }
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %hu", left_width, "Queue", cp_entry.queue);
    PROCESS_MORE("\r\n");

    len = 0;
    for (i = 0; i < 8; i++)
    {
        if (cp_entry.managed_cnpvs & (1 << (7-i)))
        {
            len += sprintf(&str[len], "%d,", i);
        }
    }
    str[len-1] = '\0';
    CLI_LIB_PrintStr_N(" %-*s : %s", left_width, "Managed CNPVs", str);
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %02X-%02X-%02X-%02X-%02X-%02X", left_width,
        "MAC Address", cp_entry.mac_address[0], cp_entry.mac_address[1],
        cp_entry.mac_address[2], cp_entry.mac_address[3],
        cp_entry.mac_address[4], cp_entry.mac_address[5]);
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %lu bytes", left_width, "Set Point",
        (unsigned long)cp_entry.set_point);
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : ", left_width, "Feedback Weight");
    switch (cp_entry.feedback_weight)
    {
    case 0:
        CLI_LIB_PrintStr_N("1/4");
        break;

    case 1:
        CLI_LIB_PrintStr_N("1/2");
        break;

    default:
        CLI_LIB_PrintStr_N("%d", 1 << (cp_entry.feedback_weight-2));
    }
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %lu bytes", left_width, "Minimum Sample Base",
        (unsigned long)cp_entry.min_sample_base);
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %llu", left_width, "Discarded Frames",
        (unsigned long long)discarded_frames);
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %llu", left_width, "Transmitted Frames",
        (unsigned long long)transmitted_frames);
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" %-*s : %llu", left_width, "Transmitted CNMs",
        (unsigned long long)qcn_stats.cpq[cp_index].qcnStatsOutCnms);
    PROCESS_MORE("\r\n");

    PROCESS_MORE("\r\n");
#endif /* #if (SYS_CPNT_CN == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_CN_ShowCp */

/* LOCAL SUBPROGRAM BODIES
 */
