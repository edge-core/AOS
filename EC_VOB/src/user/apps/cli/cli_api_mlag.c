/* =============================================================================
 * MODULE NAME : CLI_API_MLAG.C
 * PURPOSE     : Provide definitions for MLAG command line functions.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "cli_api.h"
#include "cli_def.h"
#include "cli_lib.h"
#include "cli_msg.h"
#include "cli_api_mlag.h"
#if (SYS_CPNT_MLAG == TRUE)
#include "mlag_pmgr.h"
#include "mlag_pom.h"
#include "mlag_type.h"
#endif

/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * STATIC VARIABLE DEFINITIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * EXPORTED SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - CLI_API_MLAG_Global
 * PURPOSE : Configure MLAG global status in global configuration mode
 * INPUT   : cmd_idx, arg, ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E (cli_msg.h)
 * NOTE    : None
 */
UI32_T CLI_API_MLAG_Global(UI16_T cmd_idx, char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLAG == TRUE)
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  status;

    /* BODY
     */

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W1_MLAG:
        status = MLAG_TYPE_GLOBAL_STATUS_ENABLED;
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_MLAG:
        status = MLAG_TYPE_GLOBAL_STATUS_DISABLED;
        break;

    default:
        return CLI_NO_ERROR;
    }

    if (MLAG_PMGR_SetGlobalStatus(status) != MLAG_TYPE_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_N("Failed to configure global status.\r\n");
#endif
    }
#endif /* #if (SYS_CPNT_MLAG == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_MLAG_Global */

/* FUNCTION NAME - CLI_API_MLAG_Domain
 * PURPOSE : Configure a domain in global configuration mode
 * INPUT   : cmd_idx, arg, ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E (cli_msg.h)
 * NOTE    : None
 */
UI32_T CLI_API_MLAG_Domain(UI16_T cmd_idx, char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLAG == TRUE)
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  lport;

    /* BODY
     */

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_MLAG_DOMAIN:
        if ((arg[2][0] == 'e') || (arg[2][0] == 'E'))
        {
            CLI_API_EthStatus_T verify_ret_e;
            UI32_T              verify_unit, verify_port;

            verify_unit = atoi(arg[3]);
            verify_port = atoi(strchr(arg[3], '/') + 1);
            verify_ret_e = verify_ethernet(verify_unit, verify_port, &lport);
            if (verify_ret_e != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }
        }
        else
        {
            CLI_API_TrunkStatus_T   verify_ret_t;
            UI32_T                  verify_trunk_id;

            verify_trunk_id = atoi(arg[3]);
            verify_ret_t = verify_trunk(verify_trunk_id, &lport);

            switch (verify_ret_t)
            {
            case CLI_API_TRUNK_NO_MEMBER:
                lport = verify_trunk_id + SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER - 1;
            case CLI_API_TRUNK_OK:
                break;
            default:
                display_trunk_msg(verify_ret_t, verify_trunk_id);
                return CLI_NO_ERROR;
            }
        }

        if (MLAG_PMGR_SetDomain(arg[0], lport) != MLAG_TYPE_RETURN_OK)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_N("Failed to configure a domain.\r\n");
#endif
        }

        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MLAG_DOMAIN:
        if (MLAG_PMGR_RemoveDomain(arg[0]) != MLAG_TYPE_RETURN_OK)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_N("Failed to destroy a domain.\r\n");
#endif
        }
        break;

    default:
        return CLI_NO_ERROR;
    }
#endif /* #if (SYS_CPNT_MLAG == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_MLAG_Domain */

/* FUNCTION NAME - CLI_API_MLAG_Group
 * PURPOSE : Configure a MLAG in global configuration mode
 * INPUT   : cmd_idx, arg, ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E (cli_msg.h)
 * NOTE    : None
 */
UI32_T CLI_API_MLAG_Group(UI16_T cmd_idx, char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLAG == TRUE)
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  lport;
    UI32_T  mlag_id;

    /* BODY
     */

    mlag_id = (UI32_T)atoi(arg[0]);
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_MLAG_GROUP:
        if ((arg[4][0] == 'e') || (arg[4][0] == 'E'))
        {
            CLI_API_EthStatus_T verify_ret_e;
            UI32_T              verify_unit, verify_port;

            verify_unit = atoi(arg[5]);
            verify_port = atoi(strchr(arg[5], '/') + 1);
            verify_ret_e = verify_ethernet(verify_unit, verify_port, &lport);
            if (verify_ret_e != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }
        }
        else
        {
            CLI_API_TrunkStatus_T   verify_ret_t;
            UI32_T                  verify_trunk_id;

            verify_trunk_id = atoi(arg[5]);
            verify_ret_t = verify_trunk(verify_trunk_id, &lport);

            switch (verify_ret_t)
            {
            case CLI_API_TRUNK_NO_MEMBER:
                lport = verify_trunk_id + SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER - 1;
            case CLI_API_TRUNK_OK:
                break;
            default:
                display_trunk_msg(verify_ret_t, verify_trunk_id);
                break;
            }
        }

        if (MLAG_PMGR_SetMlag(mlag_id, lport, arg[2]) != MLAG_TYPE_RETURN_OK)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_N("Failed to configure a MLAG.\r\n");
#endif
        }

        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MLAG_GROUP:
        if (MLAG_PMGR_RemoveMlag(mlag_id) != MLAG_TYPE_RETURN_OK)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_N("Failed to destroy a MLAG.\r\n");
#endif
        }
        break;

    default:
        return CLI_NO_ERROR;
    }
#endif /* #if (SYS_CPNT_MLAG == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_MLAG_Group */

/* FUNCTION NAME - CLI_API_MLAG_ShowGlobal
 * PURPOSE : Show MLAG global information in privileged execution mode
 * INPUT   : cmd_idx, arg, ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E (cli_msg.h)
 * NOTES   : None
 */
UI32_T CLI_API_MLAG_ShowGlobal(UI16_T cmd_idx, char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLAG == TRUE)
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_TYPE_DomainEntry_T domain_entry;
    UI32_T                  status;
    char                    mlag_list[MLAG_TYPE_MLAG_LIST_SIZE] = {0};
    char                    *str_list, *tmp_str;
    int                     left_margin, i, j, k;
    char                    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T                  line_num = 0;

    /* BODY
     */

    if (MLAG_POM_GetGlobalStatus(&status) != MLAG_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr_N("Failed to get global information.\r\n");
    }

    CLI_LIB_PrintStr_N(" Global Status: ");
    if (status == MLAG_TYPE_GLOBAL_STATUS_ENABLED)
    {
        PROCESS_MORE("Enabled\r\n");
    }
    else
    {
        PROCESS_MORE("Disabled\r\n");
    }

    CLI_LIB_PrintStr_N(" Domain List: ");

    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    while (MLAG_POM_GetNextDomainEntry(&domain_entry) == MLAG_TYPE_RETURN_OK)
    {
        SYSFUN_Sprintf(&buff[strlen(buff)], "%s,", domain_entry.domain_id);
    }
    if (strlen(buff) > 1)
    {
        SYSFUN_Sprintf(&buff[strlen(buff)-1], "\r\n");
        PROCESS_MORE(buff);
    }
    else
    {
        PROCESS_MORE("\r\n");
    }

    CLI_LIB_PrintStr_N(" MLAG List: ");
    left_margin = strlen(" MLAG List: ");

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    while (MLAG_POM_GetNextMlagEntry(&mlag_entry) == MLAG_TYPE_RETURN_OK)
    {
        L_CVRT_ADD_MEMBER_TO_PORTLIST(mlag_list, mlag_entry.mlag_id);
    }

    str_list = (char*)malloc(MLAG_TYPE_MAX_NBR_OF_MLAG);
    if (str_list == NULL )
    {
        return CLII_ERR_MEMORY_NOT_ENOUGH;
    }

    if (CLI_LIB_Bitmap_To_List(mlag_list, str_list, MLAG_TYPE_MAX_NBR_OF_MLAG,
            MLAG_TYPE_MAX_MLAG_ID, TRUE) == FALSE)
    {
        return CLI_ERR_INTERNAL;
    }

    tmp_str = (char*)malloc(80 - left_margin + 1);
    for (i = j = k = 0; i < strlen(str_list); i++)
    {
        if ((i - k) > (80 - left_margin))
        {
            memcpy(tmp_str, str_list+k, j-k+1);
            tmp_str[j-k+1] = 0;
            if (k == 0)
            {
                SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "%s\r\n", tmp_str);
            }
            else
            {
                SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "%*s%s\r\n",
                    left_margin, "", tmp_str);
            }
            PROCESS_MORE(buff);
            k = j + 1;
        }
        if (str_list[i] == ',')
        {
            j = i;
        }
    }

    if ((i - k) > 0)
    {
        memcpy(tmp_str, str_list+k, i-k);
        tmp_str[i-k] = 0;
        if (k == 0)
        {
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "%s\r\n", tmp_str);
        }
        else
        {
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "%*s%s\r\n", left_margin,
                "", tmp_str);
        }
        PROCESS_MORE(buff);
    }
    else
    {
        PROCESS_MORE("\r\n");
    }

    free(tmp_str);
    free(str_list);
#endif /* #if (SYS_CPNT_MLAG == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_MLAG_ShowGlobal */

/* FUNCTION NAME - CLI_API_MLAG_ShowDomain
 * PURPOSE : Show MLAG per domain information in privileged execution mode
 * INPUT   : cmd_idx, arg, ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E (cli_msg.h)
 * NOTES   : None
 */
UI32_T CLI_API_MLAG_ShowDomain(UI16_T cmd_idx, char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLAG == TRUE)
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_TYPE_DomainEntry_T domain_entry;
    SWCTRL_Lport_Type_T     lport_type;
    UI32_T                  unit, port, trunk_id;
    char                    mlag_list[MLAG_TYPE_MLAG_LIST_SIZE] = {0};
    char                    *str_list, *tmp_str;
    int                     left_margin, i, j, k;
    char                    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T                  line_num = 0;

    /* BODY
     */

    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    strncpy(domain_entry.domain_id, arg[0], MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    if (MLAG_POM_GetDomainEntry(&domain_entry) != MLAG_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr_N("Failed to get information for a domain.\r\n");
        return CLI_NO_ERROR;
    }

    lport_type = SWCTRL_POM_LogicalPortToUserPort(domain_entry.peer_link,
                        &unit, &port, &trunk_id);
    switch (lport_type)
    {
    case SWCTRL_LPORT_NORMAL_PORT:
        CLI_LIB_PrintStr_N(" Peer Link: Eth %lu/%lu", (unsigned long)unit, (unsigned long)port);
        break;

    case SWCTRL_LPORT_TRUNK_PORT:
        CLI_LIB_PrintStr_N(" Peer Link: Trunk %lu", (unsigned long)trunk_id);
        break;

    default:
        break;
    }
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" MLAG List: ");
    left_margin = strlen(" MLAG List: ");

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    strncpy(mlag_entry.domain_id, domain_entry.domain_id,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    while (MLAG_POM_GetNextMlagEntryByDomain(&mlag_entry) ==
            MLAG_TYPE_RETURN_OK)
    {
        L_CVRT_ADD_MEMBER_TO_PORTLIST(mlag_list, mlag_entry.mlag_id);
    }

    str_list = (char*)malloc(MLAG_TYPE_MAX_NBR_OF_MLAG);
    if (str_list == NULL )
    {
        return CLII_ERR_MEMORY_NOT_ENOUGH;
    }

    if (CLI_LIB_Bitmap_To_List(mlag_list, str_list, MLAG_TYPE_MAX_NBR_OF_MLAG,
            MLAG_TYPE_MAX_MLAG_ID, TRUE) == FALSE)
    {
        return CLI_ERR_INTERNAL;
    }

    tmp_str = (char*)malloc(80 - left_margin + 1);
    for (i = j = k = 0; i < strlen(str_list); i++)
    {
        if ((i - k) > (80 - left_margin))
        {
            memcpy(tmp_str, str_list+k, j-k+1);
            tmp_str[j-k+1] = 0;
            if (k == 0)
            {
                SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "%s\r\n", tmp_str);
            }
            else
            {
                SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "%*s%s\r\n",
                    left_margin, "", tmp_str);
            }
            PROCESS_MORE(buff);
            k = j + 1;
        }
        if (str_list[i] == ',')
        {
            j = i;
        }
    }

    if ((i - k) > 0)
    {
        memcpy(tmp_str, str_list+k, i-k);
        tmp_str[i-k] = 0;
        if (k == 0)
        {
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "%s\r\n", tmp_str);
        }
        else
        {
            SYSFUN_Snprintf(buff, CLI_DEF_MAX_BUFSIZE, "%*s%s\r\n", left_margin,
                "", tmp_str);
        }
        PROCESS_MORE(buff);
    }
    else
    {
        PROCESS_MORE("\r\n");
    }

    free(tmp_str);
    free(str_list);
#endif /* #if (SYS_CPNT_MLAG == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_MLAG_ShowDomain */

/* FUNCTION NAME - CLI_API_MLAG_ShowGroup
 * PURPOSE : Show MLAG per group information in privileged execution mode
 * INPUT   : cmd_idx, arg, ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E (cli_msg.h)
 * NOTES   : None
 */
UI32_T CLI_API_MLAG_ShowGroup(UI16_T cmd_idx, char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLAG == TRUE)
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;
    SWCTRL_Lport_Type_T     lport_type;
    UI32_T                  unit, port, trunk_id;
    char                    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T                  line_num = 0;

    /* BODY
     */

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    mlag_entry.mlag_id = (UI32_T)atoi(arg[0]);
    if (MLAG_POM_GetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr_N("Failed to get information for a MLAG.\r\n");
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr_N(" Domain ID: %s", mlag_entry.domain_id);
    PROCESS_MORE("\r\n");

    lport_type = SWCTRL_POM_LogicalPortToUserPort(mlag_entry.local_member,
                        &unit, &port, &trunk_id);
    switch (lport_type)
    {
    case SWCTRL_LPORT_NORMAL_PORT:
        CLI_LIB_PrintStr_N(" Local Member: Eth %lu/%lu", (unsigned long)unit, (unsigned long)port);
        break;

    case SWCTRL_LPORT_TRUNK_PORT:
        CLI_LIB_PrintStr_N(" Local Member: Trunk %lu", (unsigned long)trunk_id);
        break;

    default:
        break;
    }
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" Local State: ");
    switch (mlag_entry.local_state)
    {
    case MLAG_TYPE_STATE_INEXISTENT:
        CLI_LIB_PrintStr_N("Inactive");
        break;

    case MLAG_TYPE_STATE_DOWN:
        CLI_LIB_PrintStr_N("Down");
        break;

    case MLAG_TYPE_STATE_DORMANT:
        CLI_LIB_PrintStr_N("Dormant");
        break;

    case MLAG_TYPE_STATE_UP:
        CLI_LIB_PrintStr_N("Up");
        break;
    }
    PROCESS_MORE("\r\n");

    CLI_LIB_PrintStr_N(" Remote State: ");
    switch (mlag_entry.remote_state)
    {
    case MLAG_TYPE_STATE_INEXISTENT:
        CLI_LIB_PrintStr_N("Inactive");
        break;

    case MLAG_TYPE_STATE_DOWN:
        CLI_LIB_PrintStr_N("Down");
        break;

    case MLAG_TYPE_STATE_DORMANT:
        CLI_LIB_PrintStr_N("Dormant");
        break;

    case MLAG_TYPE_STATE_UP:
        CLI_LIB_PrintStr_N("Up");
        break;
    }
    PROCESS_MORE("\r\n");
#endif /* #if (SYS_CPNT_MLAG == TRUE) */

    return CLI_NO_ERROR;
} /* End of CLI_API_MLAG_ShowGroup */

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */
