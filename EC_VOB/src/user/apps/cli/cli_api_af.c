#include <ctype.h>
#include "cli_def.h"
#include "cli_api.h"
#include "cli_lib.h"

#include "cli_tbl.h"
#include "af_pmgr.h"


/* [no] discard {cdp | pvst}
 */
UI32_T CLI_API_AF_Discard_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_APP_FILTER == TRUE)
    UI32_T  i;
    UI32_T  max_port_num = 0;

    AF_TYPE_ErrorCode_T result;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_DISCARD:

        if (NULL == arg[0])
        {
            return CLI_ERR_INTERNAL;
        }

        for (i = 1; i <= max_port_num; i++)
        {
            if(!SWCTRL_POM_UIUserPortExisting(ctrl_P->CMenu.unit_id, i))
                continue;

            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                switch (arg[0][0])
                {
                    case 'c':
                    case 'C':
                        result = AF_PMGR_SetPortCdpStatus(i, AF_TYPE_DISCARD);
                        break;

                    case 'p':
                    case 'P':
                        result = AF_PMGR_SetPortPvstStatus(i, AF_TYPE_DISCARD);
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }

                if (AF_TYPE_SUCCESS != result)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set discard status on port %lu.\r\n",i);
#endif
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_DISCARD:

        if (NULL == arg[0])
        {
            return CLI_ERR_INTERNAL;
        }

        for (i = 1; i <= max_port_num; i++)
        {
            if(!SWCTRL_POM_UIUserPortExisting(ctrl_P->CMenu.unit_id, i))
                continue;

            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                switch (arg[0][0])
                {
                    case 'c':
                    case 'C':
                        result = AF_PMGR_SetPortCdpStatus(i, AF_TYPE_DEFAULT);
                        break;

                    case 'p':
                    case 'P':
                        result = AF_PMGR_SetPortPvstStatus(i, AF_TYPE_DEFAULT);
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }

                if (AF_TYPE_SUCCESS != result)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set discard status on port %lu.\r\n",i);
#endif
                }
            }
        }
        break;
    }

#endif /* SYS_CPNT_APP_FILTER */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_AF_Show_Discard(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    enum
    {
        AF_SUMMARY_TBL_PORT = 0,
        AF_SUMMARY_TBL_CDP_STATUS,
        AF_SUMMARY_TBL_PVST_STATUS
    };

    UI32_T unit, port;
    UI32_T  line_num = 0, max_port_num = 0, l_port = 0;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char    eth_name[10] = {0};
    char    status_str[30];

    AF_TYPE_STATUS_T status;

    CLI_TBL_Object_T tb;
    BOOL_T  is_inherit = TRUE;

    int rc;

    CLI_TBL_Temp_T summary_tbl[] =
    {
        {AF_SUMMARY_TBL_PORT,        8,  CLI_TBL_ALIGN_LEFT},
        {AF_SUMMARY_TBL_CDP_STATUS,  7,  CLI_TBL_ALIGN_LEFT},
        {AF_SUMMARY_TBL_PVST_STATUS, 7,  CLI_TBL_ALIGN_LEFT},
    };

    CLI_TBL_InitWithBuf(&tb, buff, sizeof(buff));
    CLI_TBL_SetColIndirect(&tb, summary_tbl, sizeof(summary_tbl)/sizeof(summary_tbl[0]));
    CLI_TBL_SetLineNum(&tb, line_num);

    CLI_TBL_SetColTitle(&tb, AF_SUMMARY_TBL_PORT,        "Port");
    CLI_TBL_SetColTitle(&tb, AF_SUMMARY_TBL_CDP_STATUS,  "CDP");
    CLI_TBL_SetColTitle(&tb, AF_SUMMARY_TBL_PVST_STATUS, "PVST");
    CLI_TBL_Print(&tb);

    CLI_TBL_SetLine(&tb);
    CLI_TBL_Print(&tb);

    for (unit = 0; STKTPLG_POM_GetNextUnit(&unit); )
    {
        max_port_num = SWCTRL_POM_UIGetUnitPortNumber(unit);
        for (port = 1; port <= max_port_num; ++ port)
        {
            if(!SWCTRL_POM_UIUserPortExisting(unit, port))
                continue;

            if( SWCTRL_LPORT_NORMAL_PORT != SWCTRL_POM_UIUserPortToIfindex(unit, port, &l_port, &is_inherit))
            {
                continue;
            }

            sprintf(eth_name,"Eth %1lu/%2lu", (unsigned long)unit, (unsigned long)port);

            CLI_TBL_SetColText(&tb, AF_SUMMARY_TBL_PORT, eth_name);

            AF_PMGR_GetPortCdpStatus(l_port, &status);
            if (AF_TYPE_DEFAULT == status)
            {
                strncpy(status_str, "Default", sizeof(status_str) - 1);
                status_str[sizeof(status_str) - 1] = '\0';
            }
            else if (AF_TYPE_DISCARD == status)
            {
                strncpy(status_str, "Discard", sizeof(status_str) - 1);
                status_str[sizeof(status_str) - 1] = '\0';
            }
            else
            {
                strncpy(status_str, "Unknown", sizeof(status_str) - 1);
                status_str[sizeof(status_str) - 1] = '\0';
            }

            CLI_TBL_SetColTitle(&tb, AF_SUMMARY_TBL_CDP_STATUS,  status_str);

            AF_PMGR_GetPortPvstStatus(l_port, &status);
            if (AF_TYPE_DEFAULT == status)
            {
                strncpy(status_str, "Default", sizeof(status_str) - 1);
                status_str[sizeof(status_str) - 1] = '\0';
            }
            else if (AF_TYPE_DISCARD == status)
            {
                strncpy(status_str, "Discard", sizeof(status_str) - 1);
                status_str[sizeof(status_str) - 1] = '\0';
            }
            else
            {
                strncpy(status_str, "Unknown", sizeof(status_str) - 1);
                status_str[sizeof(status_str) - 1] = '\0';
            }

            CLI_TBL_SetColTitle(&tb, AF_SUMMARY_TBL_PVST_STATUS, status_str);

            rc = CLI_TBL_Print(&tb);
            if (CLI_TBL_PRINT_RC_SUCCESS != rc)
            {
                return CLI_TBL_PRINT_FAIL_RC_TO_CLI(rc);
            }
        }
    }

    return CLI_NO_ERROR;
}

