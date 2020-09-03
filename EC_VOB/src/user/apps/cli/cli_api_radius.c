#include <stdio.h>
#include "cli_api.h"

#if (SYS_CPNT_RADIUS == TRUE)
#include "cli_api_radius.h"
#include "radius_pmgr.h"
#include "radius_pom.h"
#endif

#if (SYS_CPNT_AAA == TRUE)
#include "cli_tbl.h"
#include "aaa_pom.h"
#endif

/************************************<<RADIUS>>*****************************/
/*change mode*/
/*execution*/
UI32_T CLI_API_Show_Radius_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RADIUS == TRUE)
    RADIUS_Server_Host_T server_host;
    UI32_T server_index = 0;
    UI32_T line_num = 0;
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    UI8_T  ip[18];

    memset(&server_host, 0, sizeof(RADIUS_Server_Host_T));

    PROCESS_MORE("\r\n");
    PROCESS_MORE("Remote RADIUS Server Configuration: \r\n");
    PROCESS_MORE("\r\n");
    PROCESS_MORE("Global Settings: \r\n");

    SYSFUN_Sprintf(buff, " Authentication Port Number : %lu\r\n", (unsigned long)RADIUS_POM_Get_Server_Port());
    PROCESS_MORE(buff);

#if(SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    /*Acct-port*/
    {
        UI32_T acct_port;

        if(RADIUS_POM_GetServerAcctPort(&acct_port)!=TRUE)
        {
            PROCESS_MORE("Fail to get accounting port\r\n");
        }
        else
        {
            SYSFUN_Sprintf(buff, " Accounting Port Number     : %lu\r\n", (unsigned long)acct_port);
            PROCESS_MORE(buff);
        }
    }
#endif

    /*radius server retransmit*/
    SYSFUN_Sprintf(buff, " Retransmit Times           : %lu\r\n", (unsigned long)RADIUS_POM_Get_Retransmit_Times());
    PROCESS_MORE(buff);

    /*radius server timeout*/
    SYSFUN_Sprintf(buff, " Request Timeout            : %lu\r\n", (unsigned long)RADIUS_POM_Get_Request_Timeout());
    PROCESS_MORE(buff);

    /*line*/
    PROCESS_MORE("\r\n");

    while(RADIUS_POM_GetNext_Server_Host(&server_index, &server_host) == TRUE)
    {
        SYSFUN_Sprintf(buff, "Server %lu: \r\n", (unsigned long)server_host.server_index);
        PROCESS_MORE(buff);

        L_INET_Ntoa(server_host.server_ip, ip);
        SYSFUN_Sprintf(buff, " Server IP Address          : %s\r\n", ip);
        PROCESS_MORE(buff);

        /*radius server port*/
        SYSFUN_Sprintf(buff, " Authentication Port Number : %lu\r\n", (unsigned long)server_host.server_port);
        PROCESS_MORE(buff);

#if(SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
        SYSFUN_Sprintf(buff, " Accounting Port Number     : %lu\r\n", (unsigned long)server_host.acct_port);
        PROCESS_MORE(buff);
#endif

        /*radius server retransmit*/
        SYSFUN_Sprintf(buff, " Retransmit Times           : %lu\r\n", (unsigned long)server_host.retransmit);
        PROCESS_MORE(buff);

        /*radius server timeout*/
        SYSFUN_Sprintf(buff, " Request Timeout            : %lu\r\n", (unsigned long)server_host.timeout);
        PROCESS_MORE(buff);

        /*line*/
        PROCESS_MORE("\r\n");
    }

#if (SYS_CPNT_AAA == TRUE)
    {
        BOOL_T is_first_index;
        AAA_RadiusGroupEntryInterface_T aaa_group_entry;
        AAA_RadiusEntryInterface_T aaa_entry;

        enum
        {
            TBL_GROUP_NAME = 0,
            TBL_MEMBER_INDEX
        };

        CLI_TBL_Object_T tb;
        CLI_TBL_Temp_T group_tbl[] =
        {
            {TBL_GROUP_NAME,    25, CLI_TBL_ALIGN_LEFT},
            {TBL_MEMBER_INDEX,  13, CLI_TBL_ALIGN_LEFT},
        };
        int rc;

        memset(&aaa_group_entry, 0, sizeof(aaa_group_entry));
        memset(&aaa_entry, 0, sizeof(aaa_entry));

        PROCESS_MORE("RADIUS Server Group: \r\n");

        CLI_TBL_InitWithBuf(&tb, buff, sizeof(buff));
        CLI_TBL_SetColIndirect(&tb, group_tbl, sizeof(group_tbl)/sizeof(group_tbl[0]));
        CLI_TBL_SetLineNum(&tb, line_num);

        CLI_TBL_SetColTitle(&tb, TBL_GROUP_NAME,    "Group Name");
        CLI_TBL_SetColTitle(&tb, TBL_MEMBER_INDEX,  "Member Index");
        CLI_TBL_Print(&tb);

        CLI_TBL_SetLine(&tb);
        CLI_TBL_Print(&tb);

        while (AAA_POM_GetNextRadiusGroupEntry(&aaa_group_entry) == TRUE)
        {
            #define LOG10(x) \
            (\
             (x < 10L)          ? 1 :   \
             (x < 100L)         ? 2 :   \
             (x < 1000L)        ? 3 :   \
             (x < 10000L)       ? 4 :   \
             (x < 100000L)      ? 5 :   \
             (x < 1000000L)     ? 6 :   \
             (x < 10000000L)    ? 7 :   \
             (x < 100000000L)   ? 8 :   \
             (x < 1000000000L)  ? 9 : 10\
            )

            enum
            {
                SERVER_NUMBER_STR_LEN = LOG10(SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS),
                SEAPER_STR_LEN = sizeof(", ") -1,
                SERVER_LIST_STR_LEN = (SERVER_NUMBER_STR_LEN+SEAPER_STR_LEN)* SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS
            };

            char  index_string[ SERVER_NUMBER_STR_LEN +1] = {0};
            char  server_list_string[SERVER_LIST_STR_LEN +1] = {0};

            aaa_entry.radius_index = 0;
            is_first_index = TRUE;
            while(AAA_POM_GetNextRadiusEntry(aaa_group_entry.group_index, &aaa_entry) == TRUE)
            {
                sprintf(index_string,"%lu",aaa_entry.radius_server_index);

                if(is_first_index==TRUE)
                {
                    strcpy(server_list_string, index_string);
                    is_first_index = FALSE;
                }
                else
                {
                    strcat(server_list_string,", ");
                    strcat(server_list_string, index_string);
                }
            }

            CLI_TBL_SetColText(&tb, TBL_GROUP_NAME, aaa_group_entry.group_name);
            CLI_TBL_SetColText(&tb, TBL_MEMBER_INDEX, server_list_string);
            rc = CLI_TBL_Print(&tb);
            if (CLI_TBL_PRINT_RC_SUCCESS != rc)
            {
                return CLI_TBL_PRINT_FAIL_RC_TO_CLI(rc);
            }
        }
    }
#endif /* SYS_CPNT_AAA */

#endif /* SYS_CPNT_RADIUS */
   return CLI_NO_ERROR;
}

/*configuration*/
UI32_T CLI_API_Radius_Server_ServerIndex(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RADIUS == TRUE ||SYS_CPNT_RADIUS_AUTHENTICATION == TRUE)

    UI32_T server_index, port_no = 0, retryval = 0, timeout = 0, ip_addr;
    char   *auth_key = NULL;
    RADIUS_Server_Host_T server_host;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    UI32_T acct_port = 0;
#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)*/


    memset(&server_host, 0, sizeof(RADIUS_Server_Host_T));
    if(arg[0]!=NULL)
        server_index = atoi(arg[0]);
    else
        return CLI_ERR_INTERNAL;

    if(arg[2]!=NULL)
        CLI_LIB_AtoIp(arg[2], (UI8_T*)&ip_addr);

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_RADIUSSERVER:
            if((arg[3]!=NULL) && (arg[4]!=NULL))
            {
                if (arg[3][0] == 'a' || arg[3][0] == 'A')
                {
                    if (arg[3][1] == 'u' || arg[3][0] == 'U') /*auth-port */
                    {
                        port_no = atoi(arg[4]);
                    }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
                    else /*acct-port*/
                    {
                        acct_port = atoi(arg[4]);
                    }
#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)*/
                }
                else if (arg[3][0] == 't' || arg[3][0] == 'T')
                {
                    timeout = atoi(arg[4]);
                }
                else if (arg[3][0] == 'r' || arg[3][0] == 'R')
                {
                    retryval = atoi(arg[4]);
                }
                else if (arg[3][0] == 'k' || arg[3][0] == 'K')
                {
                    auth_key = arg[4];
                }
            }

            /*Layer 5*/
            if((arg[5]!=NULL) && (arg[6]!=NULL))
            {
                if (arg[5][0] == 't' || arg[5][0] == 'T')
                {
                    timeout = atoi(arg[6]);
                }
                else if (arg[5][0] == 'r' || arg[5][0] == 'R')
                {
                    retryval = atoi(arg[6]);
                }
                else if (arg[5][0] == 'k' || arg[5][0] == 'K')
                {
                    auth_key = arg[6];
                }
                else if (arg[5][0] == 'a' || arg[5][0] == 'A')
                {
                    if (arg[5][1] == 'u' || arg[5][1] == 'U')   /*auth-port */
                    {
                        port_no = atoi(arg[6]);
                    }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
                    else if(arg[5][1] == 'c' || arg[5][1] == 'C')/*acct-port*/
                    {
                        acct_port = atoi(arg[6]);
                    }
#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)*/
                }
            }

            /*Layer 7*/
            if((arg[7]!=NULL) && (arg[8]!=NULL))
            {
                if (arg[7][0] == 'r' || arg[7][0] == 'R')
                {
                    retryval = atoi(arg[8]);
                }
                else if (arg[7][0] == 'k' || arg[7][0] == 'K')
                {
                    auth_key = arg[8];
                }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
                else if(arg[7][0] == 'a' || arg[7][0] == 'A') /*acct-port*/
                {
                    acct_port = atoi(arg[8]);
                }
#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)*/
                else if (arg[7][0] == 't' || arg[7][0] == 'T') /*timeout*/
                {
                    timeout = atoi(arg[8]);
                }
            }

            /*Layer 9*/
            if((arg[9]!=NULL) && (arg[10]!=NULL))
            {
                if (arg[9][0] == 'k' || arg[9][0] == 'K')
                {
                    auth_key = arg[10];
                }
                else if (arg[9][0] == 't' || arg[9][0] == 'T')/*timeout*/
                {
                    timeout = atoi(arg[10]);
                }
                else if (arg[9][0] == 'r' || arg[9][0] == 'R') /*retransmit*/
                {
                    retryval = atoi(arg[10]);
                }
            }

            /*Layer 11*/
            if((arg[11]!=NULL) && (arg[12]!=NULL))
            {
                if (arg[11][0] == 'k' || arg[11][0] == 'K') /*key*/
                {
                    auth_key = arg[12];
                }
                else if(arg[11][0] == 'r' || arg[11][0] == 'R') /*key*/
                {
                    retryval = atoi(arg[12]);
                }
            }

            /*Layer 13*/
            if((arg[13]!=NULL) && (arg[14]!=NULL))
            {
                if (arg[13][0] == 'k' || arg[13][0] == 'K') /*key*/
                {
                    auth_key = arg[14];
                }
            }

            server_host.server_index = server_index;
            server_host.server_ip = ip_addr;
            server_host.server_port = port_no;
            server_host.retransmit = retryval;
            server_host.timeout = timeout;
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
            server_host.acct_port = acct_port;
#endif /*#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)*/

        /* ASF3528B-F2-FLF-TER-00041
         * the ARG_PRIVILEGE_XXX are incorrect in NEW codegen tool
         * CLI do not decode key in Parse_Arguments and decode here
         */
            if (auth_key != NULL)
            {
                /* if key decryption is failed
                 */
                if (FALSE == CLI_LIB_ConvertSecretKeyByLBase64(ctrl_P, auth_key,
                        (char *)server_host.secret, SYS_ADPT_RADIUS_SECRET_KEY_MAX_LENGTH + 1))
                {
                    break;
                }
            }

            if (RADIUS_PMGR_Set_Server_Host(server_index, &server_host) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set RADIUS Server host server_index\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_RADIUSSERVER:
            if (RADIUS_PMGR_Destroy_Server_Host(server_index) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to destroy RADIUS Server host server_index\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_RADIUS == TRUE ||SYS_CPNT_RADIUS_AUTHENTICATION == TRUE) */

    return CLI_NO_ERROR;
}


UI32_T CLI_API_Radius_Server_Host(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{/*
#if (SYS_CPNT_RADIUS == TRUE ||SYS_CPNT_RADIUS_AUTHENTICATION == TRUE)
   UI32_T ip_addr;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_RADIUSSERVER_HOST:
      CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_RADIUSSERVER_HOST:
      ip_addr = RADIUS_Default_Server_IP;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if (!RADIUS_MGR_Set_Server_IP(ip_addr))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set RADIUS server ip\r\n");
#endif
   }
#endif */
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Radius_Server_Key(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RADIUS == TRUE ||SYS_CPNT_RADIUS_AUTHENTICATION == TRUE)
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_RADIUSSERVER_KEY:
        {
            /* ASF3528B-F2-FLF-TER-00041
             * the ARG_PRIVILEGE_XXX are incorrect in NEW codegen tool
             * CLI do not decode key in Parse_Arguments and decode here
             */
            char    secret_key_ar[SYS_ADPT_RADIUS_SECRET_KEY_MAX_LENGTH + 1];

            /* if key decryption is failed
             */
            if(arg[0]!=NULL)
            {
                if (FALSE == CLI_LIB_ConvertSecretKeyByLBase64(ctrl_P, arg[0],
                    secret_key_ar, SYS_ADPT_RADIUS_SECRET_KEY_MAX_LENGTH + 1))
                {
                    break;
                }
            }
            else
                return CLI_ERR_INTERNAL;

            if (!RADIUS_PMGR_Set_Server_Secret((UI8_T *)secret_key_ar))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                 CLI_LIB_PrintStr("Failed to set RADIUS server key\r\n");
#endif
            }
        }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_RADIUSSERVER_KEY:
            if (!RADIUS_PMGR_Set_Server_Secret((UI8_T *)RADIUS_TYPE_DEFAULT_SERVER_SECRET))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set RADIUS server key\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Radius_Server_Port(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RADIUS == TRUE ||SYS_CPNT_RADIUS_AUTHENTICATION == TRUE)
    UI32_T port_no;

    switch(cmd_idx)
    {
        /*maggie liu, 2009-03-09*/
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_RADIUSSERVER_AUTHPORT:
            if(arg[0]!=NULL)
                port_no = atoi(arg[0]);
            else
                return CLI_ERR_INTERNAL;

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_RADIUSSERVER_AUTHPORT:
            port_no = SYS_DFLT_RADIUS_AUTH_CLIENT_SERVER_PORT_NUMBER;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (!RADIUS_PMGR_Set_Server_Port(port_no))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set RADIUS server port\r\n");
#endif
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Radius_Server_Retransmit(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RADIUS == TRUE ||SYS_CPNT_RADIUS_AUTHENTICATION == TRUE)
    UI32_T retryval;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_RADIUSSERVER_RETRANSMIT:
            if(arg[0]!=NULL)
                retryval = atoi(arg[0]);
            else
                return CLI_ERR_INTERNAL;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_RADIUSSERVER_RETRANSMIT:
            retryval = SYS_DFLT_RADIUS_AUTH_CLIENT_ACCESS_RETRANSMISSIONS;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (!RADIUS_PMGR_Set_Retransmit_Times(retryval))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set RADIUS server retransmit\r\n");
#endif
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Radius_Server_Timeout(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RADIUS == TRUE ||SYS_CPNT_RADIUS_AUTHENTICATION == TRUE)
    UI32_T timeout;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_RADIUSSERVER_TIMEOUT:
            if(arg[0]!=NULL)
                timeout = atoi(arg[0]);
            else
                return CLI_ERR_INTERNAL;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_RADIUSSERVER_TIMEOUT:
            timeout = SYS_DFLT_RADIUS_AUTH_CLIENT_TIMEOUTS;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (!RADIUS_PMGR_Set_Request_Timeout(timeout))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set RADIUS server timeout\r\n");
#endif
    }

#endif
    return CLI_NO_ERROR;
}


UI32_T CLI_API_Radius_Server_Acct_Port(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    UI32_T acct_port;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_RADIUSSERVER_ACCTPORT:
            if(arg[0]!=NULL)
                acct_port = atoi(arg[0]);
            else
                return CLI_ERR_INTERNAL;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_RADIUSSERVER_ACCTPORT:
            acct_port = SYS_DFLT_RADIUS_ACC_CLIENT_SERVER_PORT_NUMBER;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
    if (RADIUS_PMGR_SetServerAcctPort(acct_port) != TRUE)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set RADIUS server accounting port\r\n");
#endif
    }
#endif /* SYS_CPNT_RADIUS_ACCOUNTING */

    return CLI_NO_ERROR;
}

