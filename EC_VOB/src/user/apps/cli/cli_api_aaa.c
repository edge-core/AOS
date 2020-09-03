#include "sys_cpnt.h"
//#include "sys_adpt.h"
#include "cli_api.h"
#include "cli_lib.h"
#if (SYS_CPNT_AAA == TRUE)
#include "aaa_mgr.h"
#include "aaa_pom.h"
#include "aaa_pmgr.h"
#endif /*#if (SYS_CPNT_AAA == TRUE)*/
#include "sys_time.h"
#if 0
typedef enum CLI_AAA_ServerGroupType_E
{
    CLI_GROUP_RADIUS = 1,
    CLI_GROUP_TACACS_PLUS,
    CLI_GROUP_UNKNOWN /* for AAA_AccListEntryInterface_T, a method may map to a non-existed group */
} CLI_AAA_ServerGroupType_T;
#endif
typedef enum CLI_AAA_ShowFilterType_E
{
    CLI_SHOW_FILTE_ALL = 1,
    CLI_SHOW_FILTE_USER,
    CLI_SHOW_FILTE_INTERFACE
} CLI_AAA_ShowFilterType_T;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
static UI32_T show_accounting_commands_level(UI32_T priv_lvl, UI32_T line_num)
{
    AAA_AccListEntryInterface_T aaa_acc_list_entry_if;
    char                        buff[100] = {0};
    AAA_AccCommandEntry_T       cmd_entry;

    memset(&aaa_acc_list_entry_if, 0, sizeof(AAA_AccListEntryInterface_T));

    /*Key*/
    aaa_acc_list_entry_if.client_type = AAA_CLIENT_TYPE_COMMANDS;
    aaa_acc_list_entry_if.list_index  = 0; /*get first entry*/
    while(AAA_POM_GetNextAccListEntryFilterByClientType(&aaa_acc_list_entry_if) == TRUE)
    {
        if(aaa_acc_list_entry_if.priv_lvl != priv_lvl)
            continue;

        snprintf(buff, sizeof(buff), "Accounting Type : Commands %lu\r\n", aaa_acc_list_entry_if.priv_lvl);
        buff[ sizeof(buff)-1 ] = '\0';
        PROCESS_MORE_FUNC(buff);

        /*method list*/
        snprintf(buff, sizeof(buff), "  Method List   : %s\r\n", aaa_acc_list_entry_if.list_name);
        buff[ sizeof(buff)-1 ] = '\0';
        PROCESS_MORE_FUNC(buff);

        /*group list*/
        snprintf(buff, sizeof(buff), "  Group List    : %s\r\n", aaa_acc_list_entry_if.group_name);
        buff[ sizeof(buff)-1 ] = '\0';
        PROCESS_MORE_FUNC(buff);

        /*Interface list*/
        sprintf(buff,"  Interface     :");

        cmd_entry.priv_lvl = aaa_acc_list_entry_if.priv_lvl;
        cmd_entry.exec_type= AAA_EXEC_TYPE_CONSOLE;
        AAA_POM_GetAccCommandEntryInterface(&cmd_entry);

        if(strcmp(aaa_acc_list_entry_if.list_name, cmd_entry.list_name) == 0)
        {
            strcat(buff," Console");
            /*add ',' if vty and console map the same list */
            cmd_entry.priv_lvl = aaa_acc_list_entry_if.priv_lvl;
            cmd_entry.exec_type= AAA_EXEC_TYPE_VTY;
            AAA_POM_GetAccCommandEntryInterface(&cmd_entry);

            if(strcmp(aaa_acc_list_entry_if.list_name, cmd_entry.list_name) == 0)
            {
                strcat(buff,",");
            }
        }

        cmd_entry.priv_lvl = aaa_acc_list_entry_if.priv_lvl;
        cmd_entry.exec_type= AAA_EXEC_TYPE_VTY;
        AAA_POM_GetAccCommandEntryInterface(&cmd_entry);

        if(strcmp(aaa_acc_list_entry_if.list_name, cmd_entry.list_name) == 0)
        {
            strcat(buff," VTY");
        }
        strcat(buff,"\r\n");
        PROCESS_MORE_FUNC(buff);
        PROCESS_MORE_FUNC("\r\n");
    }/*end while(AAA_MGR_GetNextAccListEntry(&aaa_acc_list_entry_if) == TRUE)*/

    return line_num;
}

static UI32_T show_accounting_commands(UI32_T line_num)
{
    UI32_T priv_lvl;
    for(priv_lvl = 0; priv_lvl < SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE; priv_lvl++)
    {
        line_num = show_accounting_commands_level(priv_lvl, line_num);
        if ((line_num == JUMP_OUT_MORE) || (line_num == EXIT_SESSION_MORE))
            break;
    }
    return line_num;
}
#endif /* #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE) */

#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
/*static UI32_T show_accounting_by_type()*/
static UI32_T show_accounting_by_type(UI16_T client_type)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)
    AAA_AccListEntryInterface_T aaa_acc_list_entry_if;
    AAA_AccExecEntry_T acc_exec_entry;
    AAA_QueryAccDot1xPortListResult_T aaa_query_accdot1x_port_list;
    UI32_T  line_num = 0;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char    list_name_console[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1] = {0};
    char    list_name_vty[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1] = {0};

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    UI32_T max_port_num;
    UI32_T i, j, port_counter = 0, ifindex;
    BOOL_T  is_first_line = TRUE;
    UI32_T  max_num_one_line = 8;
    CLI_API_EthStatus_T         verify_ret;
    UI32_T  verify_unit, verify_port;

#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)*/

    /*stacking*/
    UI32_T current_max_unit = 0;

    memset(&aaa_acc_list_entry_if, 0, sizeof(AAA_AccListEntryInterface_T));
    memset(&aaa_query_accdot1x_port_list, 0, sizeof(AAA_QueryAccDot1xPortListResult_T));
    memset(&acc_exec_entry, 0, sizeof(AAA_AccExecEntry_T));

#if (SYS_CPNT_STACKING == TRUE)
    STKTPLG_POM_GetNumberOfUnit(&current_max_unit);
#else
    current_max_unit = 1;
#endif


#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

    /*DOT1X*/
    if(client_type == AAA_CLIENT_TYPE_DOT1X || client_type == 10)
    {
        PROCESS_MORE_FUNC("Accounting Type : dot1x\r\n");
        /*Key*/
        aaa_acc_list_entry_if.client_type = AAA_CLIENT_TYPE_DOT1X;
        aaa_acc_list_entry_if.list_index = 0; /*get first entry*/
        while(AAA_POM_GetNextAccListEntryFilterByClientType(&aaa_acc_list_entry_if) == TRUE)
        {
            /*method list*/
            snprintf(buff, sizeof(buff), "  Method List   : %s\r\n", aaa_acc_list_entry_if.list_name);
            buff[ sizeof(buff)-1 ] = '\0';
            PROCESS_MORE_FUNC(buff);

            /*group list*/
            snprintf(buff, sizeof(buff), "  Group List    : %s\r\n", aaa_acc_list_entry_if.group_name);
            buff[ sizeof(buff)-1 ] = '\0';
            PROCESS_MORE_FUNC(buff);

            /*Interface list*/
            CLI_LIB_PrintStr("  Interface     :");
            is_first_line = TRUE;
            port_counter = 0;
            aaa_query_accdot1x_port_list.list_index = aaa_acc_list_entry_if.list_index;

            if (AAA_POM_QueryAccDot1xPortList(&aaa_query_accdot1x_port_list) == TRUE)
            {
                for (j = 1; j <= current_max_unit; j++) /*stacking*/
                {
                    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
                    for (i = 1 ; i <= max_port_num ; i++)
                    {
                        verify_unit = j;
                        verify_port = i;

                        if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
                            continue; /*1. not present; 2. trunk member; 3. unknown port*/

                        if(aaa_query_accdot1x_port_list.port_list[ifindex-1] == '1')
                        {
                            port_counter++;
                            if (port_counter % max_num_one_line == 1)
                            {
                                if(is_first_line)
                                {
                                    is_first_line = FALSE;
                                }
                                else
                                {
                                    CLI_LIB_PrintStr_1("%13s"," ");
                                }
                            }
                            CLI_LIB_PrintStr_2(" Eth %lu/%-2lu", j, i);
                            if (port_counter % max_num_one_line == 0)
                            {
                                PROCESS_MORE_FUNC("\r\n");
                            }
                        }
                        else
                        {
                            continue;
                        }
                    }
                }/*for stacking unit(j)*/
            }

            if ( (port_counter % max_num_one_line != 0) || (port_counter == 0) )
            {
                PROCESS_MORE_FUNC("\r\n");
            }

            PROCESS_MORE_FUNC("\r\n");
        }/*end while(AAA_MGR_GetNextAccListEntry(&aaa_acc_list_entry_if) == TRUE)*/
    }/*end if(client_type == AAA_CLIENT_TYPE_DOT1X || client_type == 10) */

#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)*/

    /*EXEC*/
    if(client_type == AAA_CLIENT_TYPE_EXEC || client_type == 10)
    {
        PROCESS_MORE_FUNC("Accounting Type : EXEC\r\n");
        /*get console and vty's list name*/
        while(AAA_POM_GetNextAccExecEntry(&acc_exec_entry) == TRUE)
        {
            if(acc_exec_entry.exec_type == AAA_EXEC_TYPE_CONSOLE)
            {
                strcpy(list_name_console, acc_exec_entry.list_name);
            }
            else
            {
                strcpy(list_name_vty, acc_exec_entry.list_name);
            }
        }

        memset(&aaa_acc_list_entry_if, 0, sizeof(AAA_AccListEntryInterface_T));
        /*Key*/
        aaa_acc_list_entry_if.client_type = AAA_CLIENT_TYPE_EXEC;
        aaa_acc_list_entry_if.list_index = 0; /*get first entry*/
        while(AAA_POM_GetNextAccListEntryFilterByClientType(&aaa_acc_list_entry_if) == TRUE)
        {
            /*method list*/
            snprintf(buff, sizeof(buff), "  Method List   : %s\r\n", aaa_acc_list_entry_if.list_name);
            buff[ sizeof(buff)-1 ] = '\0';
            PROCESS_MORE_FUNC(buff);

            /*group list*/
            snprintf(buff, sizeof(buff), "  Group List    : %s\r\n", aaa_acc_list_entry_if.group_name);
            buff[ sizeof(buff)-1 ] = '\0';
            PROCESS_MORE_FUNC(buff);

            /*Interface list*/
            CLI_LIB_PrintStr("  Interface     :");

            if(strcmp((char *)aaa_acc_list_entry_if.list_name, list_name_console) == 0)
            {
                CLI_LIB_PrintStr(" Console");
                /*add ',' if vty and console map the same list */
                if(strcmp(list_name_vty, list_name_console) == 0)
                {
                    CLI_LIB_PrintStr(",");
                }
            }

            if(strcmp(aaa_acc_list_entry_if.list_name, list_name_vty) == 0)
            {
                CLI_LIB_PrintStr(" VTY");
            }

            PROCESS_MORE_FUNC("\r\n");
            PROCESS_MORE_FUNC("\r\n");
        }/*end while(AAA_MGR_GetNextAccListEntry(&aaa_acc_list_entry_if) == TRUE)*/
    }
#endif /*#if (SYS_CPNT_ACCOUNTING == TRUE)*/

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    /* COMMANDS */
    if(client_type == AAA_CLIENT_TYPE_COMMANDS || client_type == 10)
    {
        show_accounting_commands(line_num);
    }
#endif /* #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE) */

    return CLI_NO_ERROR;
}
#endif

#if (SYS_CPNT_ACCOUNTING == TRUE)
/*static UI32_T show_accounting_statistics_one_entry ()*/
static UI32_T show_accounting_statistics_one_entry(AAA_AccUserInfoInterface_T *aaa_acc_user_info_if)
{
    UI32_T unit, port, trunk_id;
    UI32_T seconds, diff_time;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;

    /*get time*/
    SYS_TIME_GetRealTimeBySec(&seconds);
    diff_time = seconds - aaa_acc_user_info_if->accounting_start_time;

    /*Dot1x*/
    if(aaa_acc_user_info_if->client_type == AAA_CLIENT_TYPE_DOT1X)
    {
        /*get unit and port*/
        SWCTRL_POM_LogicalPortToUserPort(aaa_acc_user_info_if->ifindex, &unit, &port, &trunk_id);
        /*display*/
        PROCESS_MORE_FUNC("Accounting Type : dot1x\r\n");
        snprintf(buff, sizeof(buff), "  Username      : %s\r\n", aaa_acc_user_info_if->user_name);
        buff[ sizeof(buff)-1 ] = '\0';
        PROCESS_MORE_FUNC(buff);

        snprintf(buff, sizeof(buff), "  Incoming Port : eth %lu/%lu\r\n", unit, port);
        buff[ sizeof(buff)-1 ] = '\0';
        PROCESS_MORE_FUNC(buff);
    }
    else /*exec*/
    {
        /*display*/
        PROCESS_MORE_FUNC("Accounting Type : EXEC\r\n");
        snprintf(buff, sizeof(buff), "  Username      : %s\r\n", aaa_acc_user_info_if->user_name);
        buff[ sizeof(buff)-1 ] = '\0';
        PROCESS_MORE_FUNC(buff);

        /*diagnose come from console or telnet*/
        if(aaa_acc_user_info_if->ifindex == 0) /*console*/
        {
            snprintf(buff, sizeof(buff), "  Incoming Port : Console\r\n");
            buff[ sizeof(buff)-1 ] = '\0';
        }
        else
        {
            snprintf(buff, sizeof(buff), "  Incoming Port : VTY %lu\r\n", aaa_acc_user_info_if->ifindex - 1);
            buff[ sizeof(buff)-1 ] = '\0';
        }
        PROCESS_MORE_FUNC(buff);
    }

    snprintf(buff, sizeof(buff), "  Time elapsed since connected : %02ld:%02ld:%02ld\r\n", diff_time/3600, (diff_time/60)%60, diff_time%60);
    buff[ sizeof(buff)-1 ] = '\0';
    PROCESS_MORE_FUNC(buff);
    PROCESS_MORE_FUNC("\r\n");

    return CLI_NO_ERROR;
}
#endif /*#if (SYS_CPNT_ACCOUNTING == TRUE)*/

#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
/*static UI32_T show_accounting_statistics_by_type ()*/
static UI32_T show_accounting_statistics_by_type(UI16_T client_type, CLI_AAA_ShowFilterType_T filter_type, char *arg[], UI32_T ifindex)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)
    AAA_AccUserInfoInterface_T aaa_acc_user_info_if;
    UI32_T qty;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;

    memset(&aaa_acc_user_info_if, 0, sizeof(AAA_AccUserInfoInterface_T));
    /*get the number of total entries for title */
    switch (filter_type)
    {
        case CLI_SHOW_FILTE_ALL:
            switch (client_type)
            {
                case AAA_CLIENT_TYPE_DOT1X:
                    AAA_PMGR_GetAccUserEntryQtyFilterByType(AAA_CLIENT_TYPE_DOT1X, &qty);
                    break;

                case AAA_CLIENT_TYPE_EXEC:
                    AAA_PMGR_GetAccUserEntryQtyFilterByType(AAA_CLIENT_TYPE_EXEC, &qty);
                    break;

                case 10:
                    AAA_PMGR_GetAccUserEntryQty(&qty);
                    break;
            }
            break;
        case CLI_SHOW_FILTE_USER:
            AAA_PMGR_GetAccUserEntryQtyFilterByNameAndType(arg[3], AAA_CLIENT_TYPE_DOT1X, &qty);
            break;
        case CLI_SHOW_FILTE_INTERFACE:
            AAA_PMGR_GetAccUserEntryQtyFilterByPort(ifindex, &qty);
            break;
    }

    /*display title*/
    sprintf(buff, "Total Entries : %lu\r\n", qty);
    PROCESS_MORE_FUNC(buff);

    switch (filter_type)
    {
        case CLI_SHOW_FILTE_ALL:
            /*dot1x*/
            if((client_type == AAA_CLIENT_TYPE_DOT1X) || (client_type == 10))
            {
                aaa_acc_user_info_if.client_type = AAA_CLIENT_TYPE_DOT1X;
                while(AAA_PMGR_GetNextAccUserEntryFilterByType(&aaa_acc_user_info_if) == TRUE)
                {
                    show_accounting_statistics_one_entry(&aaa_acc_user_info_if);
                }
            }

            /*exec*/
            if((client_type == AAA_CLIENT_TYPE_EXEC) || (client_type == 10))
            {
                memset(&aaa_acc_user_info_if, 0, sizeof(AAA_AccUserInfoInterface_T));
                aaa_acc_user_info_if.client_type = AAA_CLIENT_TYPE_EXEC;
                while(AAA_PMGR_GetNextAccUserEntryFilterByType(&aaa_acc_user_info_if) == TRUE)
                {
                    show_accounting_statistics_one_entry(&aaa_acc_user_info_if);
                }
            }
            break;

        case CLI_SHOW_FILTE_USER:
            aaa_acc_user_info_if.client_type = AAA_CLIENT_TYPE_DOT1X;
            aaa_acc_user_info_if.user_index = 0;
            strcpy(aaa_acc_user_info_if.user_name, arg[3]);
            while(AAA_PMGR_GetNextAccUserEntryFilterByNameAndType(&aaa_acc_user_info_if) == TRUE)
            {
                show_accounting_statistics_one_entry(&aaa_acc_user_info_if);
            }
            break;

        case CLI_SHOW_FILTE_INTERFACE:
            aaa_acc_user_info_if.ifindex = ifindex;
            aaa_acc_user_info_if.user_index = 0;
            while(AAA_PMGR_GetNextAccUserEntryFilterByPort(&aaa_acc_user_info_if) == TRUE)
            {
                show_accounting_statistics_one_entry(&aaa_acc_user_info_if);
            }
            break;
    }
#endif /*#if (SYS_CPNT_ACCOUNTING == TRUE)*/
    return CLI_NO_ERROR;
}

#endif

#if (SYS_CPNT_ACCOUNTING == TRUE)
static UI32_T accounting_exec(AAA_ExecType_T exec_type, char *arg[])
{
    AAA_AccExecEntry_T aaa_exec_entry;

    memset(&aaa_exec_entry, 0, sizeof(aaa_exec_entry));

    aaa_exec_entry.exec_type = exec_type;
    strncpy(aaa_exec_entry.list_name, arg[0], sizeof(aaa_exec_entry.list_name)-1);
    aaa_exec_entry.list_name[sizeof(aaa_exec_entry.list_name)-1] = '\0';

    if (AAA_PMGR_SetAccExecEntry(&aaa_exec_entry) != TRUE)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
    }

    return CLI_NO_ERROR;
}

static UI32_T no_accounting_exec(AAA_ExecType_T exec_type)
{
    if (AAA_PMGR_DisableAccExecEntry(exec_type) != TRUE)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
    }

    return CLI_NO_ERROR;
}
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
static UI32_T accounting_command(AAA_ExecType_T exec_type, char *arg[])
{
    AAA_AccCommandEntry_T aaa_cmd_entry;

    memset(&aaa_cmd_entry, 0, sizeof(aaa_cmd_entry));

    aaa_cmd_entry.priv_lvl  = atoi(arg[0]);
    aaa_cmd_entry.exec_type = exec_type;
    strncpy(aaa_cmd_entry.list_name, arg[1], sizeof(aaa_cmd_entry.list_name)-1);
    aaa_cmd_entry.list_name[sizeof(aaa_cmd_entry.list_name)-1] = '\0';

    if (AAA_PMGR_SetAccCommandEntry(&aaa_cmd_entry) != TRUE)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
    }

    return CLI_NO_ERROR;
}

static UI32_T no_accounting_command(AAA_ExecType_T exec_type, char *arg[])
{
    AAA_AccCommandEntry_T aaa_cmd_entry;

    memset(&aaa_cmd_entry, 0, sizeof(aaa_cmd_entry));

    aaa_cmd_entry.priv_lvl  = atoi(arg[0]);
    aaa_cmd_entry.exec_type = exec_type;

    if (AAA_PMGR_DisableAccCommandEntry(&aaa_cmd_entry) != TRUE)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
    }

    return CLI_NO_ERROR;
}
#endif /* #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE) */

#if (SYS_CPNT_AUTHORIZATION == TRUE)
static UI32_T authorization_exec(AAA_ExecType_T exec_type, char *arg[])
{
    AAA_AuthorExecEntry_T aaa_author_exec_entry;

    memset(&aaa_author_exec_entry, 0, sizeof(aaa_author_exec_entry));

    aaa_author_exec_entry.exec_type = exec_type;
    strncpy(aaa_author_exec_entry.list_name, arg[0], sizeof(aaa_author_exec_entry.list_name)-1);
    aaa_author_exec_entry.list_name[sizeof(aaa_author_exec_entry.list_name)-1] = '\0';

    if (AAA_PMGR_SetAuthorExecEntry(&aaa_author_exec_entry) != TRUE)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
    }

    return CLI_NO_ERROR;
}

static UI32_T no_authorization_exec(AAA_ExecType_T exec_type)
{
    if (AAA_PMGR_DisableAuthorExecEntry(exec_type) != TRUE)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
    }

    return CLI_NO_ERROR;
}
#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

/*command: [no] aaa accounting dot1x {default|list-name} {start-stop} group ... (create list-name and map group name)*/
UI32_T CLI_API_Aaa_Accounting_Dot1x(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RADIUS_ACCOUNTING==TRUE)
    AAA_AccListEntryInterface_T aaa_acc_list_entry;
    AAA_WarningInfo_T warning;
    char  group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME+1] = {0};
    char  list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME+1] = {0};
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T nShowError = 1;

    memset(&aaa_acc_list_entry, 0, sizeof(AAA_AccListEntryInterface_T));

    /*entry->list_name, entry->group_name, entry->working_mode*/
    strcpy(aaa_acc_list_entry.list_name, arg[0]);
    if(arg[3] != NULL)
    {
        CLI_API_Conver_Lower_Case(arg[3], group_name);
        if(strcmp(group_name, "radius") == 0) /*radius*/
        {
            strcpy(aaa_acc_list_entry.group_name, "radius");
        }
        else
        {
            strcpy(aaa_acc_list_entry.group_name, arg[3]);
        }
        aaa_acc_list_entry.working_mode = ACCOUNTING_START_STOP;
        aaa_acc_list_entry.client_type  = AAA_CLIENT_TYPE_DOT1X;
    }

    /*Check string if valid*/
    /*L_STBLID_StringIsAsciiPrint*/

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_AAA_ACCOUNTING_DOT1X:
            CLI_API_Conver_Lower_Case(arg[0], list_name);
            if(strcmp(list_name, "default") == 0) /*default*/
            {
                /*set default*/
                if(AAA_PMGR_SetDefaultList(&aaa_acc_list_entry) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set default list.\r\n");
#endif
                }
            }
            else
            {
                /*set list-name*/
                if(AAA_PMGR_SetAccListEntry(&aaa_acc_list_entry) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
                }
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_AAA_ACCOUNTING_DOT1X:
            if(AAA_PMGR_DestroyAccListEntry(arg[0], AAA_CLIENT_TYPE_DOT1X, &warning) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to remove list name.\r\n");
#endif
            }
            else
            {
                switch (warning.warning_type)
                {
                    case AAA_LIST_REF2_BAD_GROUP:
                        sprintf(buff, "Method list references a non-existent or removed group");
                        break;
                    case AAA_GROUP_REF2_BAD_SERVER:
                        sprintf(buff, "Server group references a non-existent or removed server host");
                        break;
                    case AAA_GROUP_HAS_NO_ENTRY:
                        sprintf(buff, "Server group doesn't have any entry");
                        break;
                    case AAA_ACC_DOT1X_REF2_BAD_LIST:
                        sprintf(buff, "Accounting dot1x references a non-existent or removed method-list");
                        break;
                    case AAA_ACC_EXEC_REF2_BAD_LIST:
                        sprintf(buff, "Accounting exec references a non-existent or removed method-list");
                        break;
                    case AAA_NO_WARNING:
                    default:
                        nShowError = 0;
                        break;
                }
                if(nShowError == 1)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("%s.\r\n", buff);
#endif
                }
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING==TRUE)*/
    return CLI_NO_ERROR;
}

/*command: [no] aaa accounting exec {default|list-name} {start-stop} group ... (create list-name and map group name)*/
UI32_T CLI_API_Aaa_Accounting_Exec(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ACCOUNTING==TRUE)
    AAA_AccListEntryInterface_T aaa_acc_list_entry;
    AAA_WarningInfo_T warning;
    char  group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME+1] = {0};
    char  list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME+1] = {0};
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T nShowError = 1;

    memset(&aaa_acc_list_entry, 0, sizeof(AAA_AccListEntryInterface_T));

    /*entry->list_name, entry->group_name, entry->working_mode*/
    strcpy(aaa_acc_list_entry.list_name, arg[0]);
    if(arg[3] != NULL)
    {
        CLI_API_Conver_Lower_Case(arg[3], group_name);
        if(strcmp(group_name, "radius") == 0)       /*radius*/
        {
            strcpy(aaa_acc_list_entry.group_name, "radius");
        }
        else if(strcmp(group_name, "tacacs+") == 0) /*tacacs+*/
        {
            strcpy(aaa_acc_list_entry.group_name, "tacacs+");
        }
        else
        {
             strcpy(aaa_acc_list_entry.group_name, arg[3]);
        }
        aaa_acc_list_entry.working_mode = ACCOUNTING_START_STOP;
        aaa_acc_list_entry.client_type  = AAA_CLIENT_TYPE_EXEC;
    }
    /*Check string if valid*/
    /*L_STBLID_StringIsAsciiPrint*/

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_AAA_ACCOUNTING_EXEC:
            CLI_API_Conver_Lower_Case(arg[0], list_name);
            if(strcmp(list_name, "default") == 0) /*default*/
            {
                /*set default*/
                if(AAA_PMGR_SetDefaultList(&aaa_acc_list_entry) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set default list.\r\n");
#endif
                }
            }
            else
            {
                /*set list-name*/
                if(AAA_PMGR_SetAccListEntry(&aaa_acc_list_entry) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
                }
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_AAA_ACCOUNTING_EXEC:
            if(AAA_PMGR_DestroyAccListEntry(arg[0], AAA_CLIENT_TYPE_EXEC, &warning) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to remove list name.\r\n");
#endif
            }
            else
            {
                switch (warning.warning_type)
                {
                    case AAA_LIST_REF2_BAD_GROUP:
                        sprintf(buff, "Method list references a non-existent or removed group");
                        break;
                    case AAA_GROUP_REF2_BAD_SERVER:
                        sprintf(buff, "Server group references a non-existent or removed server host");
                        break;
                    case AAA_GROUP_HAS_NO_ENTRY:
                        sprintf(buff, "Server group doesn't have any entry");
                        break;
                    case AAA_ACC_DOT1X_REF2_BAD_LIST:
                        sprintf(buff, "Accounting dot1x references a non-existent or removed method-list");
                        break;
                    case AAA_ACC_EXEC_REF2_BAD_LIST:
                        sprintf(buff, "Accounting exec references a non-existent or removed method-list");
                        break;
                    case AAA_NO_WARNING:
                    default:
                        nShowError = 0;
                        break;
                }
                if(nShowError == 1)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("%s.\r\n", buff);
#endif
                }
            }
            break;

         default:
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_ACCOUNTING==TRUE)*/
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
/* if src_name == "radius" then dest_name = "radius"
 * else if src_name == "tacacs+" then dest_name = "tacacs+"
 * else dest_name = src_name
 */
static void copy_group_name(char *dest_name, char *src_name)
{
    char    lower_case_src_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME+1] = {0};

    CLI_API_Conver_Lower_Case(src_name, lower_case_src_name);

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    if(strcmp(lower_case_src_name, "radius") == 0) /*radius*/
    {
        strcpy(dest_name, "radius");
    }
    else
#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)*/

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    if(strcmp(lower_case_src_name, "tacacs+") == 0) /*tacacs+*/
    {
        strcpy(dest_name, "tacacs+");
    }
    else
#endif

    {
        strcpy(dest_name, src_name);
    }
}
#endif /* SYS_CPNT_ACCOUNTING_COMMAND == TRUE */

/*command: [no] aaa accounting commands level {default|list-name} start-stop group ... (create list-name and map group name)*/
/*argidx=                                0           1                2       3    4                                       */
UI32_T CLI_API_Aaa_Accounting_Commands(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    AAA_AccListEntryInterface_T aaa_acc_list_entry;
    AAA_WarningInfo_T           warning;
    char                        list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME+1] = {0};
    char                        buff[100] = {0};
    UI32_T                      nShowError = 1;

    memset(&aaa_acc_list_entry, 0, sizeof(AAA_AccListEntryInterface_T));

    /*Check string if valid*/
    /*L_STBLID_StringIsAsciiPrint*/

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_AAA_ACCOUNTING_COMMANDS:

            /*entry->list_name, entry->group_name, entry->working_mode*/
            strcpy(aaa_acc_list_entry.list_name, arg[1]);

            copy_group_name(aaa_acc_list_entry.group_name, arg[4]);

            aaa_acc_list_entry.working_mode = ACCOUNTING_START_STOP;
            aaa_acc_list_entry.client_type  = AAA_CLIENT_TYPE_COMMANDS;
            aaa_acc_list_entry.priv_lvl     = atoi(arg[0]);

            CLI_API_Conver_Lower_Case(arg[1], list_name);
            if(strcmp(list_name, "default") == 0) /*default*/
            {
                /*set default*/
                if(AAA_PMGR_SetDefaultList(&aaa_acc_list_entry) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set default list.\r\n");
#endif
                }
            }
            else
            {
                /*set list-name*/
                if(AAA_PMGR_SetAccListEntry(&aaa_acc_list_entry) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
                }
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_AAA_ACCOUNTING_COMMANDS:

            aaa_acc_list_entry.working_mode = ACCOUNTING_START_STOP;
            aaa_acc_list_entry.client_type  = AAA_CLIENT_TYPE_COMMANDS;
            aaa_acc_list_entry.priv_lvl     = atoi(arg[0]);
            strcpy(aaa_acc_list_entry.list_name,arg[1]);

            if (AAA_PMGR_DestroyAccListEntry2(&aaa_acc_list_entry, &warning) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to remove list name.\r\n");
#endif
            }
            else
            {
                switch (warning.warning_type)
                {
                    case AAA_LIST_REF2_BAD_GROUP:
                        sprintf(buff, "Method list references a non-existent or removed group");
                        break;
                    case AAA_GROUP_REF2_BAD_SERVER:
                        sprintf(buff, "Server group references a non-existent or removed server host");
                        break;
                    case AAA_GROUP_HAS_NO_ENTRY:
                        sprintf(buff, "Server group doesn't have any entry");
                        break;
                    case AAA_ACC_DOT1X_REF2_BAD_LIST:
                        sprintf(buff, "Accounting dot1x references a non-existent or removed method-list");
                        break;
                    case AAA_ACC_EXEC_REF2_BAD_LIST:
                        sprintf(buff, "Accounting exec references a non-existent or removed method-list");
                        break;
                    case AAA_ACC_COMMAND_REF2_BAD_LIST:
                        sprintf(buff, "Accounting command reference a non-existent method-list");
                        break;
                    case AAA_NO_WARNING:
                    default:
                        nShowError = 0;
                        break;
                }
                if(nShowError == 1)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("%s.\r\n", buff);
#endif
                }
            }
            break;

         default:
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

    return CLI_NO_ERROR;
}

/*command: [no] aaa group server {radius|tacacs+} group-name (create group-name)*/
UI32_T CLI_API_Aaa_Group_Server(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if( (SYS_CPNT_AAA==TRUE) && \
((SYS_CPNT_RADIUS_ACCOUNTING==TRUE) || (SYS_CPNT_TACACS_PLUS_ACCOUNTING==TRUE) || (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE)))

    AAA_RadiusGroupEntryInterface_T aaa_radius_group_entry;
    AAA_TacacsPlusGroupEntryInterface_T aaa_tacacs_plus_group_entry;

#if (SYS_CPNT_RADIUS_ACCOUNTING==TRUE)
    AAA_WarningInfo_T warning;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T nShowError = 1;
#endif  /* #if (SYS_CPNT_RADIUS_ACCOUNTING==TRUE) */

    memset(&aaa_radius_group_entry, 0, sizeof(AAA_RadiusGroupEntryInterface_T));
    memset(&aaa_tacacs_plus_group_entry, 0, sizeof(AAA_TacacsPlusGroupEntryInterface_T));
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_AAA_GROUP_SERVER:
            switch(arg[0][0])
            {
#if (SYS_CPNT_RADIUS_ACCOUNTING==TRUE)
                case 'r':
                case 'R':
                    strcpy(aaa_radius_group_entry.group_name, arg[1]);
                    /*check if existed*/
                    if(AAA_POM_GetRadiusGroupEntry_Ex(&aaa_radius_group_entry) != TRUE)
                    {
                        /*set radius group*/
                        if(AAA_PMGR_SetRadiusGroupEntry(&aaa_radius_group_entry) != TRUE)
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to set group name.\r\n");
#endif
                            return CLI_NO_ERROR;
                        }
                        /*get radius group_index and type ,then save to CMenu*/
                        if(AAA_POM_GetRadiusGroupEntry_Ex(&aaa_radius_group_entry) != TRUE)
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to get group entry.\r\n");
#endif
                            return CLI_NO_ERROR;
                        }
                    }
                    ctrl_P->CMenu.aaa_sg_group_index = aaa_radius_group_entry.group_index;
                    ctrl_P->CMenu.aaa_sg_type = CLI_GROUP_RADIUS;
                    /*change access mode*/
                    strcpy(ctrl_P->CMenu.aaa_sg_name, arg[1]);
                    ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_AAA_SG_RADIUS_MODE;
                    break;
#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING==TRUE)*/

#if ((SYS_CPNT_TACACS_PLUS_ACCOUNTING==TRUE) || (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE))
                case 't':
                case 'T':
                    strcpy(aaa_tacacs_plus_group_entry.group_name, arg[1]);
                    /*check if existed*/
                    if(AAA_POM_GetTacacsPlusGroupEntry_Ex(&aaa_tacacs_plus_group_entry) != TRUE)
                    {
                        /*set tacacs+ group*/
                        if(AAA_PMGR_SetTacacsPlusGroupEntry(&aaa_tacacs_plus_group_entry) != TRUE)
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to set group name.\r\n");
#endif
                            return CLI_NO_ERROR;
                        }
                        /*get tacacs+ group_index and type ,then save to CMenu*/
                        if(AAA_POM_GetTacacsPlusGroupEntry_Ex(&aaa_tacacs_plus_group_entry) != TRUE)
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to get group entry.\r\n");
#endif
                            return CLI_NO_ERROR;
                        }
                    }
                    ctrl_P->CMenu.aaa_sg_group_index = aaa_tacacs_plus_group_entry.group_index;
                    ctrl_P->CMenu.aaa_sg_type = CLI_GROUP_TACACS_PLUS;
                    /*change access mode*/
                    strcpy(ctrl_P->CMenu.aaa_sg_name, arg[1]);
                    ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_AAA_SG_MODE;
                    break;
#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING==TRUE)*/

            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_AAA_GROUP_SERVER:
            switch(arg[0][0])
            {
#if (SYS_CPNT_RADIUS_ACCOUNTING==TRUE)

                case 'r':
                case 'R':
                    /*remove radius group*/
                    if(AAA_PMGR_DestroyRadiusGroupEntry(arg[1], &warning) != TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to remove group name.\r\n");
#endif
                    }
                    else
                    {
                        switch (warning.warning_type)
                        {
                            case AAA_LIST_REF2_BAD_GROUP:
                                sprintf(buff, "Method list references a non-existent or removed group");
                                break;
                            case AAA_GROUP_REF2_BAD_SERVER:
                                sprintf(buff, "Server group references a non-existent or removed server host");
                                break;
                            case AAA_GROUP_HAS_NO_ENTRY:
                                sprintf(buff, "Server group doesn't have any entry");
                                break;
                            case AAA_ACC_DOT1X_REF2_BAD_LIST:
                                sprintf(buff, "Accounting dot1x references a non-existent or removed method-list");
                                break;
                            case AAA_ACC_EXEC_REF2_BAD_LIST:
                                sprintf(buff, "Accounting exec references a non-existent or removed method-list");
                                break;
                            case AAA_NO_WARNING:
                            default:
                                nShowError = 0;
                                break;
                        }
                        if(nShowError == 1)
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("%s.\r\n", buff);
#endif
                        }
                    }
                    break;
#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING==TRUE)*/

#if ((SYS_CPNT_TACACS_PLUS_ACCOUNTING==TRUE) || (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE))
                case 't':
                case 'T':
                    /*remove tacacs+ group*/
                    if(AAA_PMGR_DestroyTacacsPlusGroupEntry(arg[1], &warning) != TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to remove group name.\r\n");
#endif
                    }
                    else
                    {
                        switch (warning.warning_type)
                        {
                            case AAA_LIST_REF2_BAD_GROUP:
                                sprintf(buff, "Method list references a non-existent or removed group");
                                break;
                            case AAA_GROUP_REF2_BAD_SERVER:
                                sprintf(buff, "Server group references a non-existent or removed server host");
                                break;
                            case AAA_GROUP_HAS_NO_ENTRY:
                                sprintf(buff, "Server group doesn't have any entry");
                                break;
                            case AAA_ACC_DOT1X_REF2_BAD_LIST:
                                sprintf(buff, "Accounting dot1x references a non-existent or removed method-list");
                                break;
                            case AAA_ACC_EXEC_REF2_BAD_LIST:
                                sprintf(buff, "Accounting exec references a non-existent or removed method-list");
                                break;
                            case AAA_NO_WARNING:
                            default:
                                nShowError = 0;
                                break;
                        }
                        if(nShowError == 1)
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("%s.\r\n", buff);
#endif
                        }
                    }
                    break;
#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING==TRUE)*/

            }
            break;

         default:
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_AAA==TRUE)*/
    return CLI_NO_ERROR;
}

/*command: aaa accounting update [periodic number] (set periodic update time)*/
UI32_T CLI_API_Aaa_Accounting_Update(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if (SYS_CPNT_ACCOUNTING==TRUE)
    UI32_T intvl = SYS_DFLT_ACCOUNTING_UPDATE_INTERVAL;
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_AAA_ACCOUNTING_UPDATE:
            if (arg[1] != NULL)
                intvl = atoi(arg[1]);
            if (AAA_PMGR_SetAccUpdateInterval(intvl) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set periodic number.\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_AAA_ACCOUNTING_UPDATE:
            if (AAA_PMGR_DisableAccUpdateInterval() != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable periodic number.\r\n");
#endif
            }
            break;

         default:
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_ACCOUNTING==TRUE)*/
    return CLI_NO_ERROR;
}

/*command: accounting dot1x [default | list-name] (decide interface to use which list name to do aaa accounting)*/
UI32_T CLI_API_Accounting_Dot1x(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if (SYS_CPNT_RADIUS_ACCOUNTING==TRUE)
    BOOL_T is_inherit        =TRUE;
    UI32_T max_port_num = 0;
    UI32_T i;
    UI32_T l_port = 0;
    AAA_AccDot1xEntry_T acc_dot1x_entry;

    memset(&acc_dot1x_entry , 0, sizeof(AAA_AccDot1xEntry_T));
	max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_ACCOUNTING_DOT1X:
            for (i = 1; i <= max_port_num; i++)
            {
                if(!SWCTRL_POM_UIUserPortExisting(ctrl_P->CMenu.unit_id, i))
                    continue;

                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id,i,&l_port, &is_inherit);
                    acc_dot1x_entry.ifindex = l_port;
                    strcpy(acc_dot1x_entry.list_name, arg[0]);
                    if(AAA_PMGR_SetAccDot1xEntry(&acc_dot1x_entry) != TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
                    }

                }
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_ACCOUNTING_DOT1X:
            for (i = 1; i <= max_port_num; i++)
            {
                if(!SWCTRL_POM_UIUserPortExisting(ctrl_P->CMenu.unit_id, i))
                    continue;

                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id,i,&l_port, &is_inherit);

                    if(AAA_PMGR_DisableAccDot1xEntry(l_port) != TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
                    }

                }
            }
            break;

         default:
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING==TRUE)*/
    return CLI_NO_ERROR;
}

/*command: accounting exec [default | list-name] (decide Console to use which list name to do aaa accounting)*/
UI32_T CLI_API_Accounting_Exec_Console(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
    UI32_T  ret = CLI_NO_ERROR;

#if (SYS_CPNT_ACCOUNTING == TRUE)
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_ACCOUNTING_EXEC:
            ret = accounting_exec(AAA_EXEC_TYPE_CONSOLE, arg);
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W3_NO_ACCOUNTING_EXEC:
            ret = no_accounting_exec(AAA_EXEC_TYPE_CONSOLE);
            break;

         default:
            ret = CLI_ERR_INTERNAL;
            break;
    }
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */

    return ret;
}

/*command: accounting exec [default | list-name] (decide Telnet to use which list name to do aaa accounting)*/
UI32_T CLI_API_Accounting_Exec_Vty(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
    UI32_T  ret = CLI_NO_ERROR;

#if (SYS_CPNT_ACCOUNTING == TRUE)
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_VTY_CMD_W2_ACCOUNTING_EXEC:
            ret = accounting_exec(AAA_EXEC_TYPE_VTY, arg);
            break;

        case PRIVILEGE_CFG_LINE_VTY_CMD_W3_NO_ACCOUNTING_EXEC:
            ret = no_accounting_exec(AAA_EXEC_TYPE_VTY);
            break;

         default:
            ret = CLI_ERR_INTERNAL;
            break;
    }
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */

    return ret;
}

/*command: accounting commands level {default | list-name} (decide Console to use which list name to do aaa accounting)*/
UI32_T CLI_API_Accounting_Commands_Console(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
    UI32_T  ret = CLI_NO_ERROR;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_ACCOUNTING_COMMANDS:
            ret = accounting_command(AAA_EXEC_TYPE_CONSOLE, arg);
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W3_NO_ACCOUNTING_COMMANDS:
            ret = no_accounting_command(AAA_EXEC_TYPE_CONSOLE, arg);
            break;

         default:
            ret = CLI_ERR_INTERNAL;
            break;
    }
#endif /* #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE) */

    return ret;
}

/*command: accounting commands level {default | list-name} (decide Telnet to use which list name to do aaa accounting)*/
UI32_T CLI_API_Accounting_Commands_Vty(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
    UI32_T  ret = CLI_NO_ERROR;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_VTY_CMD_W2_ACCOUNTING_COMMANDS:
            ret = accounting_command(AAA_EXEC_TYPE_VTY, arg);
            break;

        case PRIVILEGE_CFG_LINE_VTY_CMD_W3_NO_ACCOUNTING_COMMANDS:
            ret = no_accounting_command(AAA_EXEC_TYPE_VTY, arg);
            break;

         default:
            ret = CLI_ERR_INTERNAL;
            break;
    }
#endif /* #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE) */

    return ret;
}

/*command: Show accounting [{dot1x}] [statistics [username username | interface interface-id]]*/
/*to show the current accounting setting and to print all the accounting records for actively accounted functions*/
UI32_T CLI_API_Show_Accounting(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)
    UI32_T verify_unit, verify_port;
    UI32_T ifindex = 0;
    CLI_API_EthStatus_T verify_ret;

    if (arg[0] == NULL) /*show accounting*/
    {
        show_accounting_by_type(10); /* 10 mean to include all client_type */
    }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    else if ((arg[0][0] == 'd') || (arg[0][0] == 'D')) /*show accounting dot1x ...*/
    {
        if (arg[1] == NULL) /*show accounting dot1x*/
        {
            show_accounting_by_type(AAA_CLIENT_TYPE_DOT1X);
        }
        else /*show accounting dot1x statistics ...*/
        {
            if (arg[2] == NULL) /*show accounting dot1x statistics*/
            {
                show_accounting_statistics_by_type(AAA_CLIENT_TYPE_DOT1X, CLI_SHOW_FILTE_ALL, arg, ifindex);
            }
            else if ((arg[2][0] == 'u') || (arg[2][0] == 'U')) /*show accounting dot1x statistics user username*/
            {
                show_accounting_statistics_by_type(AAA_CLIENT_TYPE_DOT1X, CLI_SHOW_FILTE_USER, arg, ifindex);
            }
            else /*show accounting dot1x statistics interface eth 1/1*/
            {
                verify_unit = atoi(arg[4]);
                verify_port = atoi(strchr(arg[4],'/') + 1);
                /*interface*/
                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) == CLI_API_ETH_OK)
                {
                    show_accounting_statistics_by_type(AAA_CLIENT_TYPE_DOT1X, CLI_SHOW_FILTE_INTERFACE, arg, ifindex);
                }
             }
        }
    }
#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */
    else if ((arg[0][0] == 'e') || (arg[0][0] == 'E')) /*show accounting exec ...*/
    {
        if (arg[1] == NULL) /*show accounting exec*/
        {
            show_accounting_by_type(AAA_CLIENT_TYPE_EXEC);
        }
        else /*show accounting exec statistics ...*/
        {
            show_accounting_statistics_by_type(AAA_CLIENT_TYPE_EXEC, CLI_SHOW_FILTE_ALL, arg, ifindex);
        }
    }
    else if ((arg[0][0] == 's') || (arg[0][0] == 'S')) /*show accounting statistics ...*/
    {
        if (arg[1] == NULL) /*show accounting statistics*/
        {
            show_accounting_statistics_by_type(10, CLI_SHOW_FILTE_ALL, arg, ifindex); /* 10 mean to include all client_type */
        }
        else if ((arg[1][0] == 'u') || (arg[1][0] == 'U')) /*show accounting statistics user username*/
        {
            show_accounting_statistics_by_type(10, CLI_SHOW_FILTE_USER, arg, ifindex); /* 10 mean to include all client_type */
        }
        else /*show accounting statistics interface eth 1/1*/
        {
            verify_unit = atoi(arg[3]);
            verify_port = atoi(strchr(arg[3],'/') + 1);
            /*interface*/
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) == CLI_API_ETH_OK)
            {
                show_accounting_statistics_by_type(10, CLI_SHOW_FILTE_INTERFACE, arg, ifindex);
            }
       }
    }
#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    else if ((arg[0][0] == 'c') || (arg[0][0] == 'C')) /*show accounting commands level ...*/
    {
        if(arg[1] == NULL)
        {
            show_accounting_by_type(AAA_CLIENT_TYPE_COMMANDS);
        }
        else
        {
            show_accounting_commands_level(atoi(arg[1]), 0);
        }
    }
#endif /* #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE) */
#endif /*#if (SYS_CPNT_ACCOUNTING == TRUE)*/
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
/*for the usage of radius vs aaa_server_group */
static UI32_T CLI_API_AAA_SG_Server_Radius(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if ( (SYS_CPNT_AAA == TRUE) && (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) )

    AAA_RadiusEntryInterface_T aaa_radius_entry;
    UI32_T ip_addr = 0;
    AAA_WarningInfo_T aaa_warning;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T nShowError = 1;

    memset(&aaa_radius_entry, 0 , sizeof(AAA_RadiusEntryInterface_T));

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_AAA_SG_CMD_W1_SERVER: /*ADD*/
            if(ctrl_P->CMenu.aaa_sg_type == CLI_GROUP_RADIUS) /*radius group*/
            {
                if(CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr) != 1) /*Server-Index*/
                {
                    aaa_radius_entry.radius_server_index = atoi(arg[0]);
                    if(AAA_PMGR_SetRadiusEntry(ctrl_P->CMenu.aaa_sg_group_index, &aaa_radius_entry, &aaa_warning) != TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to add server ip list.\r\n");
#endif
                    }
                    else
                    {
                        switch (aaa_warning.warning_type)
                        {
                            case AAA_LIST_REF2_BAD_GROUP:
                                sprintf(buff, "Method list references a non-existent or removed group");
                                break;
                            case AAA_GROUP_REF2_BAD_SERVER:
                                sprintf(buff, "Server group references a non-existent or removed server host");
                                break;
                            case AAA_GROUP_HAS_NO_ENTRY:
                                sprintf(buff, "Server group doesn't have any entry");
                                break;
                            case AAA_ACC_DOT1X_REF2_BAD_LIST:
                                sprintf(buff, "Accounting dot1x references a non-existent or removed method-list");
                                break;
                            case AAA_ACC_EXEC_REF2_BAD_LIST:
                                sprintf(buff, "Accounting exec references a non-existent or removed method-list");
                                break;
                            case AAA_NO_WARNING:
                            default:
                                nShowError = 0;
                                break;
                        }
                        if(nShowError == 1)
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("%s.\r\n", buff);
#endif
                        }
                    }
                }
                else /*IP address*/
                {
                    if(AAA_PMGR_SetRadiusEntryByIpAddress(ctrl_P->CMenu.aaa_sg_group_index, ip_addr) != TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to add server ip list.\r\n");
#endif
                    }
                }
            }

            break;

        case PRIVILEGE_CFG_AAA_SG_CMD_W2_NO_SERVER: /*REMOVE*/
            if(ctrl_P->CMenu.aaa_sg_type == CLI_GROUP_RADIUS) /*radius group*/
            {
                if(CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr) != 1) /*Server-Index*/
                {
                    aaa_radius_entry.radius_server_index = atoi(arg[0]);
                    if(AAA_PMGR_DestroyRadiusEntry(ctrl_P->CMenu.aaa_sg_group_index, &aaa_radius_entry, &aaa_warning) != TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to remove server ip list.\r\n");
#endif
                    }
                    else
                    {
                        switch (aaa_warning.warning_type)
                        {
                            case AAA_LIST_REF2_BAD_GROUP:
                                sprintf(buff, "Method list references a non-existent or removed group");
                                break;
                            case AAA_GROUP_REF2_BAD_SERVER:
                                sprintf(buff, "Server group references a non-existent or removed server host");
                                break;
                            case AAA_GROUP_HAS_NO_ENTRY:
                                sprintf(buff, "Server group doesn't have any entry");
                                break;
                            case AAA_ACC_DOT1X_REF2_BAD_LIST:
                                sprintf(buff, "Accounting dot1x references a non-existent or removed method-list");
                                break;
                            case AAA_ACC_EXEC_REF2_BAD_LIST:
                                sprintf(buff, "Accounting exec references a non-existent or removed method-list");
                                break;
                            case AAA_NO_WARNING:
                            default:
                                nShowError = 0;
                                break;
                        }
                        if(nShowError == 1)
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("%s.\r\n", buff);
#endif
                        }
                    }
                }
                else /*IP address*/
                {
                    if(AAA_PMGR_DestroyRadiusEntryByIpAddress(ctrl_P->CMenu.aaa_sg_group_index, ip_addr, &aaa_warning) != TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to remove server ip list.\r\n");
#endif
                    }
                }
            }

            break;

    }
#endif /*#if ( (SYS_CPNT_AAA == TRUE) && (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) )*/
    return CLI_NO_ERROR;
}
#endif


#if ( (SYS_CPNT_AAA == TRUE) && ((SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE) || (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE)) )
/*for the usage of tacacs+ vs aaa_server_group */
static UI32_T CLI_API_AAA_SG_Server_Tacacs_Plus(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
    AAA_TacacsPlusEntryInterface_T aaa_tacacs_entry;
    UI32_T ip_addr = 0;
    AAA_WarningInfo_T aaa_warning;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T nShowError = 1;
    memset(&aaa_tacacs_entry, 0 , sizeof(AAA_TacacsPlusEntryInterface_T));

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_AAA_SG_CMD_W1_SERVER: /*ADD*/
            if(ctrl_P->CMenu.aaa_sg_type == CLI_GROUP_TACACS_PLUS) /*tacacs+ group*/
            {
                if(CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr) != 1) /*Server-Index*/
                {
                    aaa_tacacs_entry.tacacs_server_index = atoi(arg[0]);
                    if(AAA_PMGR_SetTacacsPlusEntry(ctrl_P->CMenu.aaa_sg_group_index, &aaa_tacacs_entry, &aaa_warning) != TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to add server ip list.\r\n");
#endif
                    }
                    else
                    {
                        switch (aaa_warning.warning_type)
                        {
                            case AAA_LIST_REF2_BAD_GROUP:
                                sprintf(buff, "Method list references a non-existent or removed group");
                                break;
                            case AAA_GROUP_REF2_BAD_SERVER:
                                sprintf(buff, "Server group references a non-existent or removed server host");
                                break;
                            case AAA_GROUP_HAS_NO_ENTRY:
                                sprintf(buff, "Server group doesn't have any entry");
                                break;
                            case AAA_ACC_DOT1X_REF2_BAD_LIST:
                                sprintf(buff, "Accounting dot1x references a non-existent or removed method-list");
                                break;
                            case AAA_ACC_EXEC_REF2_BAD_LIST:
                                sprintf(buff, "Accounting exec references a non-existent or removed method-list");
                                break;
                            case AAA_NO_WARNING:
                            default:
                                nShowError = 0;
                                break;
                        }
                        if(nShowError == 1)
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("%s.\r\n", buff);
#endif
                        }
                    }
                }
                else /*IP address*/
                {
                    if(AAA_PMGR_SetTacacsPlusEntryByIpAddress(ctrl_P->CMenu.aaa_sg_group_index, ip_addr) != TRUE)
                    {
 #if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to add server ip list.\r\n");
#endif
                    }
                }
            }

            break;

        case PRIVILEGE_CFG_AAA_SG_CMD_W2_NO_SERVER: /*REMOVE*/
            if(ctrl_P->CMenu.aaa_sg_type == CLI_GROUP_TACACS_PLUS) /*tacacs+ group*/
            {
                if(CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr) != 1) /*Server-Index*/
                {
                    aaa_tacacs_entry.tacacs_server_index = atoi(arg[0]);
                    if(AAA_PMGR_DestroyTacacsPlusEntry(ctrl_P->CMenu.aaa_sg_group_index, &aaa_tacacs_entry, &aaa_warning) != TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to remove server ip list.\r\n");
#endif
                    }
                    else
                    {
                        switch (aaa_warning.warning_type)
                        {
                            case AAA_LIST_REF2_BAD_GROUP:
                                sprintf(buff, "Method list references a non-existent or removed group");
                                break;
                            case AAA_GROUP_REF2_BAD_SERVER:
                                sprintf(buff, "Server group references a non-existent or removed server host");
                                break;
                            case AAA_GROUP_HAS_NO_ENTRY:
                                sprintf(buff, "Server group doesn't have any entry");
                                break;
                            case AAA_ACC_DOT1X_REF2_BAD_LIST:
                                sprintf(buff, "Accounting dot1x references a non-existent or removed method-list");
                                break;
                            case AAA_ACC_EXEC_REF2_BAD_LIST:
                                sprintf(buff, "Accounting exec references a non-existent or removed method-list");
                                break;
                            case AAA_NO_WARNING:
                            default:
                                nShowError = 0;
                                break;
                        }
                        if(nShowError == 1)
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("%s.\r\n", buff);
#endif
                        }
                    }
                }
                else /*IP address*/
                {
                    if(AAA_PMGR_DestroyTacacsPlusEntryByIpAddress(ctrl_P->CMenu.aaa_sg_group_index, ip_addr, &aaa_warning) != TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to remove server ip list.\r\n");
#endif
                    }
                }
            }

            break;
    }

    return CLI_NO_ERROR;
}
#endif /*#if ( (SYS_CPNT_AAA == TRUE) && ((SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE) || (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE)) )*/


/*command: [no] server {server-index | list-name} (add/remove server ip list)*/
UI32_T CLI_API_AAA_SG_Server(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if( (SYS_CPNT_AAA==TRUE) && \
((SYS_CPNT_RADIUS_ACCOUNTING==TRUE) || (SYS_CPNT_TACACS_PLUS_ACCOUNTING==TRUE) || (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE)) )

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_AAA_SG_CMD_W1_SERVER: /*ADD*/
        case PRIVILEGE_CFG_AAA_SG_CMD_W2_NO_SERVER: /*REMOVE*/
#if (SYS_CPNT_RADIUS_ACCOUNTING==TRUE)
            if(ctrl_P->CMenu.aaa_sg_type == CLI_GROUP_RADIUS) /*radius group*/
            {
                CLI_API_AAA_SG_Server_Radius(cmd_idx, arg, ctrl_P);
            }
#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING==TRUE)   */

#if ((SYS_CPNT_TACACS_PLUS_ACCOUNTING==TRUE) || (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE))
            if(ctrl_P->CMenu.aaa_sg_type == CLI_GROUP_TACACS_PLUS) /*tacacs+ group*/
            {
                CLI_API_AAA_SG_Server_Tacacs_Plus(cmd_idx, arg, ctrl_P);
            }
#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING==TRUE)*/

            break;

         default:
            return CLI_ERR_INTERNAL;
    }

#endif /*#if (SYS_CPNT_AAA==TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T show_authorization_exec(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P, UI32_T line_num)
{
#if (SYS_CPNT_AUTHORIZATION == TRUE)
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char    list_name_console[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME + 1] = {0};
    char    list_name_vty[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME + 1] = {0};

    AAA_AuthorExecEntry_T aaa_author_exec_entry;
    AAA_AuthorListEntryInterface_T aaa_author_list_entry;


    memset(&aaa_author_exec_entry, 0, sizeof(AAA_AuthorExecEntry_T));
    memset(&aaa_author_list_entry, 0, sizeof(AAA_AuthorListEntryInterface_T));


    PROCESS_MORE_FUNC("Authorization Type : EXEC\r\n");
    /*get console and vty's list name*/
    while(AAA_POM_GetNextAuthorExecEntry(&aaa_author_exec_entry) == TRUE)
    {
        if(aaa_author_exec_entry.exec_type == AAA_EXEC_TYPE_CONSOLE)
        {
            strcpy(list_name_console, aaa_author_exec_entry.list_name);
        }
        else
        {
            strcpy(list_name_vty, aaa_author_exec_entry.list_name);
        }
    }

    /*Key*/
    aaa_author_list_entry.list_type.client_type = AAA_CLIENT_TYPE_EXEC;
    aaa_author_list_entry.list_index = 0; /*get first entry*/
    while(AAA_POM_GetNextAuthorListEntryFilterByClientType(&aaa_author_list_entry) == TRUE)
    {
        /*method list*/
        snprintf(buff, sizeof(buff), "  Method List   : %s\r\n", aaa_author_list_entry.list_name);
        buff[sizeof(buff)-1] = '\0';
        PROCESS_MORE_FUNC(buff);

        /*group list*/
        snprintf(buff, sizeof(buff), "  Group List    : %s\r\n", aaa_author_list_entry.group_name);
        buff[sizeof(buff)-1] = '\0';
        PROCESS_MORE_FUNC(buff);

        /*Interface list*/
        CLI_LIB_PrintStr("  Interface     :");

        if (strcmp(aaa_author_list_entry.list_name, list_name_console) == 0)
        {
            CLI_LIB_PrintStr(" Console");
            /*add ',' if vty and console map the same list */
            if(strcmp(list_name_vty, list_name_console) == 0)
            {
                CLI_LIB_PrintStr(",");
            }
        }

        if (strcmp(aaa_author_list_entry.list_name, list_name_vty) == 0)
        {
            CLI_LIB_PrintStr(" VTY");
        }

        PROCESS_MORE_FUNC("\r\n");
        PROCESS_MORE_FUNC("\r\n");
    }/*end while(AAA_MGR_GetNextAccListEntry(&aaa_acc_list_entry_if) == TRUE)*/

#endif /*#if (SYS_CPNT_AUTHORIZATION == TRUE)*/

    return line_num;
}

UI32_T show_authorization_commands_level(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P, UI32_T priv_lvl, UI32_T line_num)
{
#if  (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
    AAA_AuthorListEntryInterface_T  aaa_author_list_entry_if;
    char                            buff[CLI_DEF_MAX_BUFSIZE] = {0};
    AAA_AuthorCommandEntry_T        aaa_author_entry;

    memset(&aaa_author_list_entry_if, 0, sizeof(aaa_author_list_entry_if));

    aaa_author_list_entry_if.list_type.client_type = AAA_CLIENT_TYPE_COMMANDS;
    aaa_author_list_entry_if.list_index = 0;
    while (AAA_POM_GetNextAuthorListEntryFilterByClientType(&aaa_author_list_entry_if) == TRUE)
    {
        BOOL_T have_console = FALSE;

        if (aaa_author_list_entry_if.list_type.priv_lvl != priv_lvl)
            continue;

        snprintf(buff, sizeof(buff), "Authorization Type : Commands %lu\r\n", aaa_author_list_entry_if.list_type.priv_lvl);
        buff[sizeof(buff)-1] = '\0';
        PROCESS_MORE_FUNC(buff);

        snprintf(buff, sizeof(buff), "  Method List   : %s\r\n", aaa_author_list_entry_if.list_name);
        buff[sizeof(buff)-1] = '\0';
        PROCESS_MORE_FUNC(buff);

        snprintf(buff, sizeof(buff), "  Group List    : %s\r\n", aaa_author_list_entry_if.group_name);
        buff[sizeof(buff)-1] = '\0';
        PROCESS_MORE_FUNC(buff);

        sprintf(buff,"  Interface     :");

        AAA_POM_GetAuthorCommandEntry(aaa_author_list_entry_if.list_type.priv_lvl,
                                      AAA_EXEC_TYPE_CONSOLE,
                                      &aaa_author_entry);

        if (strcmp(aaa_author_list_entry_if.list_name, aaa_author_entry.list_name) == 0)
        {
            strcat(buff," Console");
            have_console = TRUE;
        }

        AAA_POM_GetAuthorCommandEntry(aaa_author_list_entry_if.list_type.priv_lvl,
                                      AAA_EXEC_TYPE_VTY,
                                      &aaa_author_entry);

        if (strcmp(aaa_author_list_entry_if.list_name, aaa_author_entry.list_name) == 0)
        {
            if (have_console)
            {
                strcat(buff, ",");
            }

            strcat(buff," VTY");
        }

        strcat(buff,"\r\n");
        PROCESS_MORE_FUNC(buff);
        PROCESS_MORE_FUNC("\r\n");
    }
#endif /* #if  (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */
    return line_num;
}

/* command: show authorization commands [level]
 * argidx=                          0      1
 */
UI32_T show_authorization_commands(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P, UI32_T line_num)
{
#if  (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
    UI32_T priv_lvl;

    if (NULL == arg[1])
    {
        for (priv_lvl = 0; priv_lvl < SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE; priv_lvl++)
        {
            line_num = show_authorization_commands_level(cmd_idx, arg, ctrl_P, priv_lvl, line_num);

            if (line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
        }
    }
    else
    {
        priv_lvl = atoi(arg[1]);
        line_num = show_authorization_commands_level(cmd_idx, arg, ctrl_P, priv_lvl, line_num);

        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
    }
#endif /* #if  (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */
    return line_num;
}

/* command: show authorization [exec | commands level]
 * argidx=                       0
 */
UI32_T CLI_API_Show_Authorization(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
    UI32_T line_num = 0;

    if (NULL == arg[0])
    {
        line_num = show_authorization_exec(cmd_idx, arg, ctrl_P, line_num);

        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }

        line_num = show_authorization_commands(cmd_idx, arg, ctrl_P, line_num);

        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }

    }
    else if (arg[0][0] == 'e' || arg[0][0] == 'E')
    {
        line_num = show_authorization_exec(cmd_idx, arg, ctrl_P, line_num);

        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }

    }
    else if (arg[0][0] == 'c' || arg[0][0] == 'C')
    {
        line_num = show_authorization_commands(cmd_idx, arg, ctrl_P, line_num);

        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Aaa_Authorization_Exec(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if  (SYS_CPNT_AUTHORIZATION == TRUE)

    AAA_AuthorListEntryInterface_T aaa_author_list_entry;
    AAA_ListType_T                 aaa_author_list_type;
    AAA_WarningInfo_T warning;
    char  group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME+1] = {0};
    char  list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME+1] = {0};
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T nShowError = 1;

    memset(&aaa_author_list_entry, 0, sizeof(aaa_author_list_entry));
    memset(&aaa_author_list_type, 0, sizeof(aaa_author_list_type));

    aaa_author_list_type.client_type = AAA_CLIENT_TYPE_EXEC;

    /*Check string if valid*/
    /*L_STBLID_StringIsAsciiPrint*/
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_AAA_AUTHORIZATION_EXEC:
            CLI_API_Conver_Lower_Case(arg[0], list_name);
            if(strcmp(list_name, "default") == 0) /*default*/
            {
                /*set default*/
                if(AAA_PMGR_SetAuthorDefaultList(&aaa_author_list_type, arg[2]) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set default list.\r\n");
#endif
                }
            }
            else
            {

                /*entry->list_name, entry->group_name, entry->working_mode*/
                strcpy(aaa_author_list_entry.list_name, arg[0]);
                CLI_API_Conver_Lower_Case(arg[2], group_name);
#if 0
                if(strcmp(group_name, "radius") == 0) /*radius*/
                {
                    strcpy(aaa_author_list_entry.group_name, "radius");
                }
#endif
                if(strcmp(group_name, "tacacs+") == 0) /*tacacs+*/
                {
                    strcpy(aaa_author_list_entry.group_name, "tacacs+");
                }
                else
                {
                    strcpy(aaa_author_list_entry.group_name, arg[2]);
                }

                aaa_author_list_entry.list_type.client_type = AAA_CLIENT_TYPE_EXEC;
                /*set list-name*/
                if(AAA_PMGR_SetAuthorListEntry(&aaa_author_list_entry) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
                }
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_AAA_AUTHORIZATION_EXEC:
            if(AAA_PMGR_DestroyAuthorListEntry(arg[0], &aaa_author_list_type, &warning) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to remove list name.\r\n");
#endif
            }
            else
            {
                switch (warning.warning_type)
                {
                    case AAA_LIST_REF2_BAD_GROUP:
                        sprintf(buff, "Method list references a non-existent or removed group");
                        break;
                    case AAA_GROUP_REF2_BAD_SERVER:
                        sprintf(buff, "Server group references a non-existent or removed server host");
                        break;
                    case AAA_GROUP_HAS_NO_ENTRY:
                        sprintf(buff, "Server group doesn't have any entry");
                        break;
                    case AAA_ACC_DOT1X_REF2_BAD_LIST:
                        sprintf(buff, "Accounting dot1x references a non-existent or removed method-list");
                        break;
                    case AAA_ACC_EXEC_REF2_BAD_LIST:
                        sprintf(buff, "Accounting exec references a non-existent or removed method-list");
                        break;
                    case AAA_NO_WARNING:
                    default:
                        nShowError = 0;
                        break;
                }
                if(nShowError == 1)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("%s.\r\n", buff);
#endif
                }
            }
            break;

         default:
            return CLI_ERR_INTERNAL;
    }

#endif
    return CLI_NO_ERROR;

}

UI32_T CLI_API_Authorization_Exec_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T  *ctrl_P)
{
    UI32_T  ret = CLI_NO_ERROR;

#if (SYS_CPNT_AUTHORIZATION == TRUE)
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_AUTHORIZATION_EXEC:
            ret = authorization_exec(AAA_EXEC_TYPE_CONSOLE, arg);
            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W3_NO_AUTHORIZATION_EXEC:
            ret = no_authorization_exec(AAA_EXEC_TYPE_CONSOLE);
            break;

         default:
            ret = CLI_ERR_INTERNAL;
            break;
    }
#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

    return ret;
}

UI32_T CLI_API_Authorization_Exec_Vty(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T  *ctrl_P)
{
    UI32_T  ret = CLI_NO_ERROR;

#if (SYS_CPNT_AUTHORIZATION == TRUE)
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_VTY_CMD_W2_AUTHORIZATION_EXEC:
            ret = authorization_exec(AAA_EXEC_TYPE_VTY, arg);
            break;

        case PRIVILEGE_CFG_LINE_VTY_CMD_W3_NO_AUTHORIZATION_EXEC:
            ret = no_authorization_exec(AAA_EXEC_TYPE_VTY);
            break;

         default:
            ret = CLI_ERR_INTERNAL;
            break;
    }
#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

    return ret;
}
/* command: aaa authorization commands level {default | list-name} group {tacacs+ | group-name}
 * argidx=                                0       1                  2       3
 *
 * command: no aaa authorization commands level  {default | list-name}
 * argidx=                                  0        1
 */
UI32_T CLI_API_Aaa_Authorization_Commands(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if  (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
    AAA_AuthorListEntryInterface_T aaa_author_list_entry;
    AAA_WarningInfo_T warning;
    char  group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME+1] = {0};
    char  list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME+1] = {0};
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T nShowError = 1;

    memset(&aaa_author_list_entry, 0, sizeof(aaa_author_list_entry));

    aaa_author_list_entry.list_type.client_type = AAA_CLIENT_TYPE_COMMANDS;
    aaa_author_list_entry.list_type.priv_lvl = atoi(arg[0]);

    strcpy(aaa_author_list_entry.list_name, arg[1]);
    CLI_API_Conver_Lower_Case(arg[1], list_name);

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_AAA_AUTHORIZATION_COMMANDS:

            if (strcmp(list_name, "default") == 0)
            {
                /* set default
                 */
                if (AAA_PMGR_SetAuthorDefaultList(&aaa_author_list_entry.list_type, arg[3]) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set default list.\r\n");
#endif
                }
            }
            else
            {
                CLI_API_Conver_Lower_Case(arg[3], group_name);
#if 0
                if(strcmp(group_name, "radius") == 0) /*radius*/
                {
                    strcpy(aaa_author_list_entry.group_name, "radius");
                }
#endif
                if (strcmp(group_name, "tacacs+") == 0) /*tacacs+*/
                {
                    strcpy(aaa_author_list_entry.group_name, "tacacs+");
                }
                else
                {
                    strcpy(aaa_author_list_entry.group_name, arg[3]);
                }

                if (AAA_PMGR_SetAuthorListEntry(&aaa_author_list_entry) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
                }
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_AAA_AUTHORIZATION_COMMANDS:
            if (AAA_PMGR_DestroyAuthorListEntry(aaa_author_list_entry.list_name,
                                                &aaa_author_list_entry.list_type, &warning) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to remove list name.\r\n");
#endif
            }
            else
            {
                switch (warning.warning_type)
                {
                    case AAA_LIST_REF2_BAD_GROUP:
                        sprintf(buff, "Method list references a non-existent or removed group");
                        break;
                    case AAA_GROUP_REF2_BAD_SERVER:
                        sprintf(buff, "Server group references a non-existent or removed server host");
                        break;
                    case AAA_GROUP_HAS_NO_ENTRY:
                        sprintf(buff, "Server group doesn't have any entry");
                        break;
                    case AAA_ACC_DOT1X_REF2_BAD_LIST:
                        sprintf(buff, "Accounting dot1x references a non-existent or removed method-list");
                        break;
                    case AAA_ACC_EXEC_REF2_BAD_LIST:
                        sprintf(buff, "Accounting exec references a non-existent or removed method-list");
                        break;
                    case AAA_NO_WARNING:
                    default:
                        nShowError = 0;
                        break;
                }
                if(nShowError == 1)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("%s.\r\n", buff);
#endif
                }
            }
            break;

         default:
            return CLI_ERR_INTERNAL;
    }

#endif
    return CLI_NO_ERROR;

}

/* command: authorization commands level {default|list-name}
 * argidx=                           0           1
 *
 * command: no authorization commands level
 * argidx=                              0
 */
UI32_T CLI_API_Authorization_Commands_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
    AAA_AuthorCommandEntry_T aaa_author_command_entry;

    memset(&aaa_author_command_entry, 0, sizeof(aaa_author_command_entry));

    aaa_author_command_entry.priv_lvl  = atoi(arg[0]);
    aaa_author_command_entry.exec_type = AAA_EXEC_TYPE_CONSOLE;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W2_AUTHORIZATION_COMMANDS:
            strncpy(aaa_author_command_entry.list_name, arg[1], sizeof(aaa_author_command_entry.list_name)-1);
            aaa_author_command_entry.list_name[ sizeof(aaa_author_command_entry.list_name)-1 ] = '\0';

            if (AAA_PMGR_SetAuthorCommandEntry(&aaa_author_command_entry) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
            }

            break;

        case PRIVILEGE_CFG_LINE_CONSOLE_CMD_W3_NO_AUTHORIZATION_COMMANDS:

            if (AAA_PMGR_DisableAuthorCommandEntry(aaa_author_command_entry.priv_lvl,
                                                   aaa_author_command_entry.exec_type) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
            }

            break;

         default:
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Authorization_Commands_Vty(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
    AAA_AuthorCommandEntry_T aaa_author_command_entry;

    memset(&aaa_author_command_entry, 0, sizeof(aaa_author_command_entry));

    aaa_author_command_entry.priv_lvl  = atoi(arg[0]);
    aaa_author_command_entry.exec_type = AAA_EXEC_TYPE_VTY;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_LINE_VTY_CMD_W2_AUTHORIZATION_COMMANDS:
            strncpy(aaa_author_command_entry.list_name, arg[1], sizeof(aaa_author_command_entry.list_name)-1);
            aaa_author_command_entry.list_name[ sizeof(aaa_author_command_entry.list_name)-1 ] = '\0';

            if (AAA_PMGR_SetAuthorCommandEntry(&aaa_author_command_entry) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
            }

            break;

        case PRIVILEGE_CFG_LINE_VTY_CMD_W3_NO_AUTHORIZATION_COMMANDS:

            if (AAA_PMGR_DisableAuthorCommandEntry(aaa_author_command_entry.priv_lvl,
                                                   aaa_author_command_entry.exec_type) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set list name.\r\n");
#endif
            }

            break;

         default:
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ip_Http_Authentication_AAA_Exec_Authorization(
    UI16_T cmd_idx,
    char *arg[],
    CLI_TASK_WorkingArea_T  *ctrl_P)
{
    UI32_T  ret = CLI_NO_ERROR;

#if (SYS_CPNT_AUTHORIZATION == TRUE)
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W5_IP_HTTP_AUTHENTICATION_AAA_EXECAUTHORIZATION:
            ret = authorization_exec(AAA_EXEC_TYPE_HTTP, arg);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W6_NO_IP_HTTP_AUTHENTICATION_AAA_EXECAUTHORIZATION:
            ret = no_authorization_exec(AAA_EXEC_TYPE_HTTP);
            break;

         default:
            ret = CLI_ERR_INTERNAL;
            break;
    }
#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

    return ret;
}
