#include <stdio.h>
#include "cli_api.h"

#if (SYS_CPNT_TACACS == TRUE)
#include "cli_api_tacacs.h"
#include "tacacs_pmgr.h"
#include "tacacs_pom.h"
#endif

#if (SYS_CPNT_AAA == TRUE)
#include "cli_tbl.h"
#include "aaa_pom.h"
#endif

UI32_T CLI_API_Tacacs_Server_Port(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TACACS == TRUE )
    UI32_T port = 0;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_TACACSSERVER_PORT:

            if(arg[0]!=NULL)
                port = atoi(arg[0]);
            else
                return CLI_ERR_INTERNAL;

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_TACACSSERVER_PORT:
            port = TACACS_DEFAULT_SERVER_PORT;
            break;

        default :
            return CLI_ERR_INTERNAL;
    }
    if (!TACACS_PMGR_Set_Server_Port(port))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set TACACS server port\r\n");
#endif
    }

#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Tacacs_Server_Retransmit(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TACACS_PLUS_RETRY_TIMEOUT == TRUE )
   UI32_T retransmit = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_TACACSSERVER_RETRANSMIT:
      retransmit = atoi(arg[0]);
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_TACACSSERVER_RETRANSMIT:
      retransmit = SYS_DFLT_TACACS_AUTH_CLIENT_ACCESS_RETRANSMISSIONS;
      break;

   default :
      return CLI_ERR_INTERNAL;
   }

   if (FALSE == TACACS_PMGR_SetServerRetransmit(retransmit))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set TACACS server retransmit\r\n");
#endif
   }

#endif /* #if (SYS_CPNT_TACACS_PLUS_RETRY_TIMEOUT == TRUE ) */
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Tacacs_Server_Timeout(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TACACS_PLUS_RETRY_TIMEOUT == TRUE )
   UI32_T timeout = 0;

   switch(cmd_idx)
   {
       case PRIVILEGE_CFG_GLOBAL_CMD_W2_TACACSSERVER_TIMEOUT:
          timeout = atoi(arg[0]);
          break;

       case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_TACACSSERVER_TIMEOUT:
          timeout = SYS_DFLT_TACACS_AUTH_CLIENT_TIMEOUTS;
          break;

       default :
          return CLI_ERR_INTERNAL;
   }

   if (FALSE == TACACS_PMGR_SetServerTimeout(timeout))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set TACACS server timeout\r\n");
#endif
   }

#endif /* #if (SYS_CPNT_TACACS_PLUS_RETRY_TIMEOUT == TRUE ) */
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Tacacs_Server_Key(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TACACS == TRUE )
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_TACACSSERVER_KEY:
        {
            /* ASF3528B-F2-FLF-TER-00041
             * the ARG_PRIVILEGE_XXX are incorrect in NEW codegen tool
             * CLI do not decode key in Parse_Arguments and decode here
             */
            char   secret_key_ar[MAXSIZE_tacacsServerKey + 1];

            /* if key decryption is failed
             */
            if(arg[0]!=NULL)
            {
                if (FALSE == CLI_LIB_ConvertSecretKeyByLBase64(ctrl_P, arg[0],
                    secret_key_ar, MAXSIZE_tacacsServerKey + 1))
                {
                    break;
                }
            }
            else
                return CLI_ERR_INTERNAL;


            if (!TACACS_PMGR_Set_Server_Secret((UI8_T *)secret_key_ar))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set TACACS server key\r\n");
#endif
            }
        }
             break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_TACACSSERVER_KEY:
            if (!TACACS_PMGR_Set_Server_Secret((UI8_T *)TACACS_DEFAULT_SERVER_KEY))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set default TACACS server key\r\n");
#endif
            }
            break;

        default :
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

#if 0 /*maggie liu, ES3628BT-FLF-ZZ-00052/ES3628BT-FLF-ZZ-00158/ES4827G-FLF-ZZ-00404*/
UI32_T CLI_API_Tacacs_Server_Host(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TACACS == TRUE )

/* Single TACACS server */
#if (SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER != TRUE)
    UI32_T ip_addr;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_TACACSSERVER_HOST:
            if(arg[0]!=NULL)
                CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
            else
                return CLI_ERR_INTERNAL;

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_TACACSSERVER_HOST:
            ip_addr = TACACS_DEFAULT_SERVER_IP;
            break;

        default :
            return CLI_ERR_INTERNAL;
    }

    if (!TACACS_PMGR_Set_Server_IP(ip_addr))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set TACACS server IP address\r\n");
#endif
    }


/* Multiple TACACS servers does not support "tacacs-server host" command */
/* #if (SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER == TRUE)*/
#else
    TACACS_Server_Host_T server_host;

    memset(&server_host, 0, sizeof(TACACS_Server_Host_T));
    server_host.server_port = TACACS_DEFAULT_SERVER_PORT;

    /*get server ip address*/
    if(arg[0]!=NULL)
        CLI_LIB_AtoIp(arg[0], (UI8_T*)&(server_host.server_ip));
    else
        return CLI_ERR_INTERNAL;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_TACACSSERVER_HOST:
        {
            UI8_T   *auth_key = NULL;

            /*get port number*/
            if((arg[1]!=NULL) && (arg[2]!=NULL))
            {
                if(arg[1][0] == 'p' || arg[1][0] == 'P')
                {
                    server_host.server_port = atoi(arg[2]);
                    /*get key string*/
                    if((arg[3]!=NULL) && (arg[4]!=NULL))
                    {
                        if(arg[3][0] == 'k' || arg[3][0] == 'K')
                        {
                            auth_key = arg[4];
                        }
                    }
                }
                /*get key string*/
                else if(arg[1][0] == 'k' || arg[1][0] == 'K')
                {
                    auth_key = arg[2];
                }
            }

            /* ASF3528B-F2-FLF-TER-00041
             * the ARG_PRIVILEGE_XXX are incorrect in NEW codegen tool
             * CLI do not decode key in Parse_Arguments and decode here
             */
            if (auth_key != NULL)
            {
                /* if key decryption is failed
                 */
                if (FALSE == CLI_LIB_ConvertSecretKeyByLBase64(ctrl_P, auth_key,
                    server_host.secret, MAXSIZE_tacacsServerKey + 1))
                {
                    break;
                }
            }

            /*set*/
            if(TACACS_PMGR_SetServerHostByIpAddress(&server_host) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set TACACS server host parameter\r\n");
#endif
            }
        }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_TACACSSERVER_HOST:
            if(TACACS_PMGR_Destroy_Server_Host_By_Ip_Address(server_host.server_ip) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to delete TACACS server\r\n");
#endif
   }
            break;

        default :
            return CLI_ERR_INTERNAL;
    }
#endif /* end of SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER */
#endif /* end of #if (SYS_CPNT_TACACS == TRUE )  */
    return CLI_NO_ERROR;
}
#endif

UI32_T CLI_API_Show_Tacacs_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TACACS == TRUE )
    UI32_T line_num = 0;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI8_T  ip[18];

    PROCESS_MORE("\r\n");
    PROCESS_MORE("Remote TACACS+ Server Configuration: \r\n");
    PROCESS_MORE("\r\n");
    PROCESS_MORE("Global Settings: \r\n");

/* Single TACACS server */
#if (SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER != TRUE)
    {
        UI32_T ip_addr;

        ip_addr =  TACACS_PMGR_Get_Server_IP();

        L_INET_Ntoa(ip_addr, ip);
        sprintf(buff, " Server IP Address  : %s\r\n", ip);
        PROCESS_MORE(buff);
    }
#endif /* end of #if (SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER != TRUE) */

    /* global configuration */
    sprintf(buff, " Server Port Number : %lu\r\n", (unsigned long)TACACS_POM_Get_Server_Port());
    PROCESS_MORE(buff);

#if (SYS_CPNT_TACACS_PLUS_RETRY_TIMEOUT == TRUE )
    sprintf(buff, " Retransmit Times   : %lu\r\n", (unsigned long)TACACS_POM_GetServerRetransmit());
    PROCESS_MORE(buff);
    sprintf(buff, " Timeout            : %lu\r\n", (unsigned long)TACACS_POM_GetServerTimeout());
    PROCESS_MORE(buff);
#endif /* #if (SYS_CPNT_TACACS_PLUS_RETRY_TIMEOUT == TRUE ) */

/* Multiple TACACS servers */
#if (SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER == TRUE)
    /*line*/
    PROCESS_MORE("\r\n");

    {
        TACACS_Server_Host_T server_host;
        UI32_T server_index = 0;

        memset(&server_host, 0, sizeof(TACACS_Server_Host_T));

        while(TACACS_POM_GetNext_Server_Host(&server_index, &server_host) == TRUE)
        {
            sprintf(buff, "Server %lu: \r\n", (unsigned long)server_host.server_index);
            PROCESS_MORE(buff);

            L_INET_Ntoa(server_host.server_ip, ip);
            sprintf(buff, " Server IP Address  : %s\r\n", ip);
            PROCESS_MORE(buff);

            sprintf(buff, " Server Port Number : %lu\r\n", (unsigned long)server_host.server_port);
            PROCESS_MORE(buff);

#if (SYS_CPNT_TACACS_PLUS_RETRY_TIMEOUT == TRUE )
            sprintf(buff, " Retransmit Times   : %lu\r\n", (unsigned long)server_host.retransmit);
            PROCESS_MORE(buff); 
            
            sprintf(buff, " Timeout            : %lu\r\n", (unsigned long)server_host.timeout);
            PROCESS_MORE(buff);
#endif /* #if (SYS_CPNT_TACACS_PLUS_RETRY_TIMEOUT == TRUE ) */
  
            /*line*/
            CLI_LIB_PrintStr("\r\n");
        }
    }
#endif /* #if(SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER == TRUE) */

#if (SYS_CPNT_AAA == TRUE)
    {
        BOOL_T is_first_index;
        AAA_TacacsPlusGroupEntryInterface_T aaa_group_entry;
        AAA_TacacsPlusEntryInterface_T aaa_entry;

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

        PROCESS_MORE("TACACS Server Group: \r\n");

        CLI_TBL_InitWithBuf(&tb, buff, sizeof(buff));
        CLI_TBL_SetColIndirect(&tb, group_tbl, sizeof(group_tbl)/sizeof(group_tbl[0]));
        CLI_TBL_SetLineNum(&tb, line_num);

        CLI_TBL_SetColTitle(&tb, TBL_GROUP_NAME,    "Group Name");
        CLI_TBL_SetColTitle(&tb, TBL_MEMBER_INDEX,  "Member Index");
        CLI_TBL_Print(&tb);

        CLI_TBL_SetLine(&tb);
        CLI_TBL_Print(&tb);

        while (AAA_POM_GetNextTacacsPlusGroupEntry(&aaa_group_entry) == TRUE)
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
                SERVER_NUMBER_STR_LEN = LOG10(SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS),
                SEAPER_STR_LEN = sizeof(", ") -1,
                SERVER_LIST_STR_LEN = (SERVER_NUMBER_STR_LEN+SEAPER_STR_LEN)* SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS
            };

            char  index_string[ SERVER_NUMBER_STR_LEN +1] = {0};
            char  server_list_string[SERVER_LIST_STR_LEN +1] = {0};

            aaa_entry.tacacs_index = 0;
            is_first_index = TRUE;
            while(AAA_POM_GetNextTacacsPlusEntry(aaa_group_entry.group_index, &aaa_entry) == TRUE)
            {
                sprintf(index_string,"%lu",aaa_entry.tacacs_server_index);

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

#endif /* #if (SYS_CPNT_TACACS == TRUE ) */
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Tacacs_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TACACS == TRUE && SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER == TRUE )   
    UI32_T                  nServerIndex;
    UI32_T                  ix;
    char                   *sec_key_p = NULL;
    TACACS_Server_Host_T    server_host;

    memset(&server_host, 0, sizeof(TACACS_Server_Host_T));

    /*get server index*/
    if(arg[0]!=NULL)
        nServerIndex = atoi(arg[0]);
    else
        return CLI_ERR_INTERNAL;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W1_TACACSSERVER:
        {
            /*get ip address*/
            if(arg[2]!=NULL)
                CLI_LIB_AtoIp(arg[2], (UI8_T*)&(server_host.server_ip));
            else
                return CLI_ERR_INTERNAL;

            ix = 3;
            while (arg[ix])
            {
                switch (arg[ix][0])
                {
                    case 'p': case 'P':
                        server_host.server_port = (UI32_T)atoi(arg[ix+1]);
                        break;

                    case 'k': case 'K':
                        sec_key_p = arg[ix+1];

                        /* decrypt the key when cli provision
                         */
                        if(NULL != sec_key_p)
                        {
                            if (FALSE == CLI_LIB_ConvertSecretKeyByLBase64(ctrl_P, sec_key_p,
                                (char *)server_host.secret, sizeof(server_host.secret)))
                            {
                                return CLI_ERR_INTERNAL;
                            }
                        }

                        break;

                #if (SYS_CPNT_TACACS_PLUS_RETRY_TIMEOUT == TRUE)
                    case 'r': case 'R':
                        server_host.retransmit = (UI32_T)atoi(arg[ix+1]);
                        break;

                    case 't': case 'T':
                        server_host.timeout = (UI32_T)atoi(arg[ix+1]);
                        break;
                #endif /* #if (SYS_CPNT_TACACS_PLUS_RETRY_TIMEOUT == TRUE) */

                    default:
                        return CLI_ERR_INTERNAL;
                }
                ix += 2;
            }

            /*set*/
            if(TACACS_PMGR_Set_Server_Host(nServerIndex, &server_host) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set TACACS server index parameter\r\n");
#endif
            }
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_TACACSSERVER:
        if(TACACS_PMGR_Destroy_Server_Host_By_Index(nServerIndex) != TRUE)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to delete TACACS server index parameter\r\n");
#endif
        }
        break;

    default :
        return CLI_ERR_INTERNAL;
   }
#endif /* #if (SYS_CPNT_TACACS == TRUE && SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER == TRUE) */

   return CLI_NO_ERROR;
}
