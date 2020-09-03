#include "sysfun.h"
#include <string.h>
#include <stdlib.h>
#include <sys_type.h>
#include <time.h>
 #include <stdio.h>
//#include "skt_vx.h"
//#include "socket.h"

#include "sys_dflt.h"

/*cli internal*/
#include "clis.h"
#include "cli_def.h"
#include "cli_mgr.h"
#include "cli_io.h"
//#include "cli_type.h"
#include "cli_task.h"
#include "cli_func.h"
#include "cli_main.h"
#include "cli_lib.h"
#include "cli_api.h"
#include "cli_pars.h"
#include "cli_msg.h"
#include "cli_cmd.h"
#include "cli_auth.h"
#include "cli_runcfg.h"

#include "sshd_type.h"
#include "sshd_pmgr.h"
#if(SYS_CPNT_SSH2 == TRUE)
#include "keygen_type.h"
#include "keygen_pmgr.h"
#endif


#if(SYS_CPNT_SSH2 == TRUE)
static UI32_T show_public_key(char *key_p, UI32_T ke_max_size, UI32_T line_num);
#endif


UI32_T CLI_API_Disconnect_SSH(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SSHD==TRUE)
   UI32_T cid;

   cid = atoi(arg[0]);

   if (!SSHD_PMGR_CheckSshConnection(cid))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Connection is not SSH or not existed\r\n");
#endif
      return CLI_NO_ERROR;
   }

   if (!CLI_TASK_SetKillWorkingSpaceFlag(cid + 1))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to disconnect SSH session\r\n");
#endif
   }

 #endif

   return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_IP_SSH(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T key_size = 0;

#if ( SYS_CPNT_SSH2 == TRUE )
    UI32_T major = 0;
    UI32_T minor = 0;
    if(SSHD_PMGR_GetSshdStatus() == SSHD_STATE_ENABLED)
    {
        SSHD_PMGR_GetSshServerVersion(&major, &minor);
        CLI_LIB_PrintStr_2("SSH Enabled - Version %lu.%lu\r\n", (unsigned long)major, (unsigned long)minor);
        CLI_LIB_PrintStr_2("Negotiation Timeout : %lu seconds; Authentication Retries : %lu\r\n",(unsigned long)SSHD_PMGR_GetNegotiationTimeout(),(unsigned long)SSHD_PMGR_GetAuthenticationRetries());
        SSHD_PMGR_GetServerKeySize(&key_size);
        CLI_LIB_PrintStr_1("Server Key Size     : %lu bits\r\n", (unsigned long)key_size);
    }
    else
    {
        CLI_LIB_PrintStr("SSH Disabled \r\n");
    }
#else
    CLI_LIB_PrintStr("Information of Secure Shell\r\n");
    CLI_LIB_PrintStr_1("SSH Status                 : %s\r\n", (SSHD_PMGR_GetSshdStatus() == SSHD_STATE_ENABLED) ?("Enabled"):("Disabled"));
    CLI_LIB_PrintStr_1("SSH Authentication Timeout : %lu\r\n",(unsigned long)SSHD_PMGR_GetNegotiationTimeout());
    CLI_LIB_PrintStr_1("SSH Authentication Retries : %lu\r\n",(unsigned long)SSHD_PMGR_GetAuthenticationRetries());
    SSHD_PMGR_GetServerKeySize(&key_size);
    CLI_LIB_PrintStr_1("SSH Server Key Size        : %lu bits\r\n", (unsigned long)key_size);
#endif

   return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_SSH(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

#if(SYS_CPNT_SSH2 == TRUE)
   I32_T   cid = -1, i = 0 , k = 0;
   UI32_T line_num = 0;
   char    buff[CLI_DEF_MAX_BUFSIZE]       = {0};
   char    encrypt1[SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN+1] = {0};
   char    encrypt2[SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN/2] = {0};
   char    head[SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN+1] = {0};
   UI32_T   pp = 0;
   SSHD_CONNECTION_INFO_T info;


   memset(&info, 0, sizeof(SSHD_CONNECTION_INFO_T));
   if(!SSHD_PMGR_GetNextSshConnectionEntry(&cid, &info))
   {
       CLI_LIB_PrintStr("No SSH server connections running.\r\n");
       CLI_LIB_PrintStr("\r\n");
       return CLI_NO_ERROR;
   }
   else
   {
      char nego_st[20] = {0};
      switch(info.status)
      {
      case NEGOTIATION_STARTED:
         strcpy(nego_st, "Negotiation-started");
         break;
      case AUTHENTICATION_STARTED:
         strcpy(nego_st, "Authentication-started");
         break;
      case SESSION_STARTED:
         strcpy(nego_st, "Session-started");
         break;
      default:
         break;
      }

      strcpy(head, (char *)info.cipher);
      for (i=0;i < SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN/2;i++)
      {
          if( head[i] == '|')
	  {
	      strncpy(encrypt1,head, i-1);
              for (k = 0; k < SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN/2 ; k++)
              {
                  encrypt2[k] = head[(k + i + 1) ];
              }
              pp = 1;
              break;
          }
      }

      if( pp == 0 )
      {
          strcpy(encrypt1, head);
      }

      CLI_LIB_PrintStr("Connection Version State                  Username Encryption                  \r\n");
      sprintf(buff,"  %-3lu      %3lu.%-3lu %-22s %-8s %-28s\r\n", (unsigned long)cid,  (unsigned long)info.major_version, (unsigned long)info.minor_version, nego_st , info.username, encrypt1);
      PROCESS_MORE(buff);
      line_num = 2;
      if( pp == 1 )
      {
          sprintf(buff,"                                                  %s\r\n", encrypt2);
          PROCESS_MORE(buff);
          line_num = 4;
      }

   while(SSHD_PMGR_GetNextSshConnectionEntry(&cid, &info))
   {
      char nego_st[20] = {0};

      memset(encrypt1, 0, SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN+1);
      memset(encrypt2, 0, SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN/2);
      memset(head, 0, SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN+1);
      pp = 0;

      switch(info.status)
      {
      case NEGOTIATION_STARTED:
         strcpy(nego_st, "Negotiation-started");
         break;

      case AUTHENTICATION_STARTED:
         strcpy(nego_st, "Authentication-started");
         break;

      case SESSION_STARTED:
         strcpy(nego_st, "Session-started");
         break;

      default:
         break;
      }

      strcpy(head, (char *)info.cipher);
         for (i=0;i<SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN/2;i++)
      {
          if( head[i] == '|')
	  {
	      strncpy(encrypt1,head, i-1);
               for (k = 0; k < SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN/2; k++)
              {
                  encrypt2[k] = head[(k + i + 1) ];
              }
              pp = 1;
              break;
          }
      }

      if( pp == 0 )
      {
          strcpy(encrypt1, head);
      }
         sprintf(buff,"  %-3lu      %3lu.%-3lu %-22s %-8s %-28s\r\n", (unsigned long)cid,  (unsigned long)info.major_version, (unsigned long)info.minor_version, nego_st , info.username, encrypt1);
     PROCESS_MORE(buff);
      if( pp == 1 )
      {
            sprintf(buff,"                                                  %s\r\n", encrypt2);
          PROCESS_MORE(buff);
          line_num = 4;
      }
   }
   }
#endif
   return CLI_NO_ERROR;
}


UI32_T CLI_API_IP_SSH_AuthenticationRetries(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SSHD == TRUE || SYS_CPNT_SSH2 == TRUE)
   UI32_T retries;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_SSH_AUTHENTICATIONRETRIES:
      retries = atoi(arg[0]);
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_SSH_AUTHENTICATIONRETRIES:
      retries = SSHD_DEFAULT_AUTHENTICATION_RETRIES;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if (!SSHD_PMGR_SetAuthenticationRetries(retries))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set Secure Shell authentication retry times\r\n");
#endif
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_IP_SSH_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SSHD == TRUE || SYS_CPNT_SSH2 == TRUE)
   UI32_T status;
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_SSH_SERVER:
      status = SSHD_STATE_ENABLED;
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_SSH_SERVER:
      status = SSHD_STATE_DISABLED;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if(!SSHD_PMGR_SetSshdStatus(status))
   {
      CLI_LIB_PrintStr("Failed to set Secure Shell status\r\n");
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_IP_SSH_Timeout(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SSHD == TRUE || SYS_CPNT_SSH2 == TRUE)
   UI32_T timeout;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_SSH_TIMEOUT:
      timeout = atoi(arg[0]);
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_SSH_TIMEOUT:
      timeout = SSHD_DEFAULT_NEGOTIATION_TIMEOUT;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if (!SSHD_PMGR_SetNegotiationTimeout(timeout))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set timeout of sercue shell\r\n");
#endif
   }
#endif
   return CLI_NO_ERROR;
}



/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ip_Ssh_Save_Hostkey
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "ip ssh save host-key"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Ip_Ssh_Save_Hostkey(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SSH2 ==TRUE)

    if (KEYGEN_PMGR_WriteHostKey2Flash() !=TRUE)
    {
        CLI_LIB_PrintStr("Failed to save host key to flash\r\n");
    }

#endif
    return CLI_NO_ERROR;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ip_Ssh_Serverkey_Size
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "ip ssh server-key size"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Ip_Ssh_Serverkey_Size(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SSH2 == TRUE)
    UI32_T key_size = 0;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W4_IP_SSH_SERVERKEY_SIZE:
        key_size = atoi(arg[0]);

        break;
    case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_IP_SSH_SERVERKEY_SIZE:
        key_size = SYS_DFLT_SSH_SERVER_KEY_SIZE;
        break;
    default:
        break;
    }
    if ( SSHD_PMGR_SetServerKeySize(key_size) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set the SSH server key size\r\n");
    }
#endif
    return CLI_NO_ERROR;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME -CLI_API_Delete_PublicKey
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "delete public-key"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Delete_PublicKey(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SSH2 == TRUE)
    char *username = 0;
    UI32_T key_type = KEY_TYPE_BOTH_RSA_AND_DSA;

    /* format: delete public-key username [dsa | rsa]
     */
    username = arg[0];

    if (NULL != arg[1])
    {
        switch(*arg[1])
        {
        case 'r':/* zeroize rsa */
        case 'R':
            key_type = KEY_TYPE_RSA;
            break;

        case 'd':/* zeroize dsa */
        case 'D':
            key_type = KEY_TYPE_DSA;
            break;
        }
    }

    if ( SSHD_PMGR_DeleteUserPublicKey((UI8_T *)username, key_type) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to delete public key\r\n");
    }
#endif
    return CLI_NO_ERROR;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ip_Ssh_Crypto_Hostkey_Generate
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "ip ssh crypto host-key generate"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Ip_Ssh_Crypto_Hostkey_Generate(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SSH2 == TRUE)
    UI32_T  key_type = KEY_TYPE_BOTH_RSA_AND_DSA;

    /* format: ip ssh crypto host-key generate [dsa | rsa]
     */
    if (NULL != arg[0])
    {
        switch(*arg[0])
        {
        case 'r':/* zeroize rsa */
        case 'R':
            key_type = KEY_TYPE_RSA;
            break;

        case 'd':/* zeroize dsa */
        case 'D':
            key_type = KEY_TYPE_DSA;
            break;
        }
    }

    if ( KEYGEN_PMGR_GenerateHostKeyPair(key_type) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to generate host key\r\n");
    }
#endif

    return CLI_NO_ERROR;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ip_Ssh_Crypto_Zeroize
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "ip ssh crypto host-key zeroize"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Ip_Ssh_Crypto_Zeroize(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SSH2 == TRUE)
    UI32_T  key_type = KEY_TYPE_BOTH_RSA_AND_DSA;

    /* format: ip ssh crypto zeroize [dsa | rsa]
     */
    if (NULL != arg[0])
    {
        switch(*arg[0])
        {
        case 'r':/* zeroize rsa */
        case 'R':
            key_type = KEY_TYPE_RSA;
            break;

        case 'd':/* zeroize dsa */
        case 'D':
            key_type = KEY_TYPE_DSA;
            break;
        }
    }

    if ( KEYGEN_PMGR_DeleteHostKeyPair(key_type) != TRUE )
    {
        CLI_LIB_PrintStr("Failed to delete host key\r\n");
    }
#endif

    return CLI_NO_ERROR;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_PublicKey
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show public-key"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_PublicKey(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SSH2 == TRUE)
    UI32_T line_num = 0;
    char   rsa_host_key[HOST_RSA_KEY_1024B_FILE_LENGTH + 1] = {0};
    char   dsa_host_key[HOST_DSA_KEY_1024B_FILE_LENGTH + 1] = {0};
    char   rsa_public_key[USER_RSA_PUBLIC_KEY_FILE_LENGTH + 1] = {0};
    char   dsa_public_key[USER_DSA_PUBLIC_KEY_FILE_LENGTH + 1] = {0};
    char   username[SYS_ADPT_MAX_USER_NAME_LEN+1]={0};
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

    /* format: show public key [host | user [username]]
     */
    if (NULL != arg[0])
    {
        switch(*arg[0])
        {
        case 'h':/*public-key host */
        case 'H':
            PROCESS_MORE("Host: \r\n");
            if( KEYGEN_PMGR_GetHostPublicKey((UI8_T *)rsa_host_key, (UI8_T *)dsa_host_key) ==TRUE)
            {
                PROCESS_MORE("RSA: \r\n");
                if((line_num = show_public_key(rsa_host_key, HOST_RSA_KEY_1024B_FILE_LENGTH, line_num)) == JUMP_OUT_MORE)
                {
                   return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                   return CLI_EXIT_SESSION;
                }

                PROCESS_MORE("DSA: \r\n");

                if((line_num = show_public_key(dsa_host_key, HOST_DSA_KEY_1024B_FILE_LENGTH, line_num)) == JUMP_OUT_MORE)
                {
                   return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                   return CLI_EXIT_SESSION;
                }
                PROCESS_MORE("\r\n");
            }
            break;

        case 'u':/*public-key user*/
        case 'U':
            if( arg[1] == NULL )
            {
                //PROCESS_MORE("User: \r\n");
                while( SSHD_PMGR_GetNextUserPublicKey((UI8_T *)username, (UI8_T *)rsa_public_key, (UI8_T *)dsa_public_key) ==TRUE)
                {
                    sprintf(buff, "%s: \r\n", username);
                    PROCESS_MORE(buff);
                    PROCESS_MORE("RSA: \r\n");
                    if((line_num = show_public_key(rsa_public_key, USER_RSA_PUBLIC_KEY_FILE_LENGTH, line_num)) == JUMP_OUT_MORE)
                    {
                       return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                       return CLI_EXIT_SESSION;
                    }
                    PROCESS_MORE("DSA: \r\n");

                    if((line_num = show_public_key(dsa_public_key, USER_DSA_PUBLIC_KEY_FILE_LENGTH, line_num)) == JUMP_OUT_MORE)
                    {
                       return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                       return CLI_EXIT_SESSION;
                    }
                    PROCESS_MORE("\r\n");
                }
            }
            else
            {
                /*PROCESS_MORE("%s: \r\n", arg[1]);*/
                if( SSHD_PMGR_GetUserPublicKey((UI8_T *)arg[1], (UI8_T *)rsa_public_key, (UI8_T *)dsa_public_key) == TRUE )
                {
                    sprintf(buff, "%s: \r\n", arg[1]);
                    PROCESS_MORE(buff);
                    PROCESS_MORE("RSA: \r\n");
                    if((line_num = show_public_key(rsa_public_key, USER_RSA_PUBLIC_KEY_FILE_LENGTH, line_num)) == JUMP_OUT_MORE)
                    {
                       return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                       return CLI_EXIT_SESSION;
                    }

                    PROCESS_MORE("DSA: \r\n");
                    if((line_num = show_public_key(dsa_public_key, USER_DSA_PUBLIC_KEY_FILE_LENGTH, line_num)) == JUMP_OUT_MORE)
                    {
                       return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                       return CLI_EXIT_SESSION;
                    }
                    PROCESS_MORE("\r\n");
                }
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
        }
    }
    else
    {
        PROCESS_MORE("Host: \r\n");
        if( KEYGEN_PMGR_GetHostPublicKey((UI8_T *)rsa_host_key, (UI8_T *)dsa_host_key) ==TRUE)
        {
            PROCESS_MORE("RSA: \r\n");
            if((line_num = show_public_key(rsa_host_key, HOST_RSA_KEY_1024B_FILE_LENGTH, line_num)) == JUMP_OUT_MORE)
            {
               return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
               return CLI_EXIT_SESSION;
            }
            PROCESS_MORE("DSA: \r\n");
            if((line_num = show_public_key(dsa_host_key, HOST_DSA_KEY_1024B_FILE_LENGTH, line_num)) == JUMP_OUT_MORE)
            {
               return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
               return CLI_EXIT_SESSION;
            }
            PROCESS_MORE("\r\n");
        }
        while( SSHD_PMGR_GetNextUserPublicKey((UI8_T *)username, (UI8_T *)rsa_public_key, (UI8_T *)dsa_public_key) ==TRUE)
        {
            sprintf(buff, "%s: \r\n", username);
            PROCESS_MORE(buff);
            PROCESS_MORE("RSA: \r\n");
            if((line_num = show_public_key(rsa_public_key, USER_RSA_PUBLIC_KEY_FILE_LENGTH, line_num)) == JUMP_OUT_MORE)
            {
               return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
               return CLI_EXIT_SESSION;
            }
            PROCESS_MORE("DSA: \r\n");

            if((line_num = show_public_key(dsa_public_key, USER_DSA_PUBLIC_KEY_FILE_LENGTH, line_num)) == JUMP_OUT_MORE)
            {
               return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
               return CLI_EXIT_SESSION;
            }
            PROCESS_MORE("\r\n");
        }
    }
#endif

    return CLI_NO_ERROR;
}

#if(SYS_CPNT_SSH2 == TRUE)
/*
LOCAL SUBPROGRAM
*/
#define LINE_WIDTH 78
static UI32_T show_public_key(char *key_p, UI32_T key_max_size, UI32_T line_num)
{
    UI32_T i = 0;
    UI32_T j;
    char buff[LINE_WIDTH + 3] = { 0 };

    while ('\0' != key_p[i])
    {
        buff[0] = '\0';

        for (j = 0; (j < LINE_WIDTH) || (i > key_max_size); j++, i++)
        {
            if (   ('\n' == key_p[i])
                || ('\0' == key_p[i])
                )
            {
                i++;
                break;
            }

            buff[j] = key_p[i];
        }

        buff[j] = '\r';
        buff[j + 1] = '\n';
        buff[j + 2] = '\0';

        PROCESS_MORE_FUNC(buff);
    }

    return line_num;
}
#endif
