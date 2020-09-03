#include "cli_api.h"
#include "cli_msg.h"
#include "cli_task.h"
#include "smtp_pmgr.h"
#include "l_inet.h"
#include <stdio.h>

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Logging_Sendmail
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "logging sendmail"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Logging_Sendmail(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SMTP==TRUE)

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_LOGGING_SENDMAIL:

      if ( SMTP_PMGR_EnableSmtpAdminStatus() != SMTP_RETURN_SUCCESS)
      {
         CLI_LIB_PrintStr("Failed to enable logging sendmail \r\n");
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LOGGING_SENDMAIL:
      if ( SMTP_PMGR_DisableSmtpAdminStatus() != SMTP_RETURN_SUCCESS)
      {
         CLI_LIB_PrintStr("Failed to disable logging sendmail \r\n");
      }
      break;
   default:
      break;
   }
#endif

   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Logging_Sendmail_DestinationEmail
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "logging sendmail destination-email"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Logging_Sendmail_DestinationEmail(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SMTP==TRUE)
    UI8_T  email_addr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1] = {0};
    strcpy((char *)email_addr, arg[0]);
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_LOGGING_SENDMAIL_DESTINATIONEMAIL:
       if ( SMTP_PMGR_AddSmtpDestinationEmailAddr(email_addr) != SMTP_RETURN_SUCCESS)
       {
           CLI_LIB_PrintStr("Failed to add destination-email address\r\n");
       }
       break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_LOGGING_SENDMAIL_DESTINATIONEMAIL:
       if (SMTP_PMGR_DeleteSmtpDestinationEmailAddr(email_addr) != SMTP_RETURN_SUCCESS)
       {
          CLI_LIB_PrintStr("Failed to delete destination-email address\r\n");
       }
       break;
    default:
       break;
    }
 #endif
 
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Logging_Sendmail_Host
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "logging sendmail host"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Logging_Sendmail_Host(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SMTP==TRUE)
    UI32_T ip_addr = 0;
    L_INET_Aton((UI8_T *)arg[0], &ip_addr);
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_LOGGING_SENDMAIL_HOST:
        if ( SMTP_PMGR_AddSmtpServerIPAddr(ip_addr) != SMTP_RETURN_SUCCESS )
        {
            CLI_LIB_PrintStr("Failed to add host address\r\n");
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_LOGGING_SENDMAIL_HOST:
        if ( SMTP_PMGR_DeleteSmtpServerIPAddr(ip_addr) != SMTP_RETURN_SUCCESS )
        {
            CLI_LIB_PrintStr("Failed to delete host address\r\n");
        }
        break;
    default:
        break;
   }
#endif

   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Logging_Sendmail_Level
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "logging sendmail level"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Logging_Sendmail_Level(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SMTP==TRUE)
    UI32_T level = 0;
    level = atoi(arg[0]);
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_LOGGING_SENDMAIL_LEVEL:
        if ( SMTP_PMGR_SetEmailSeverityLevel(level) != SMTP_RETURN_SUCCESS)
        {
            CLI_LIB_PrintStr("Failed to set logging sendmail level\r\n");
        }
        break;
       
        //ADD: daniel, 2008.08.07
    case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_LOGGING_SENDMAIL_LEVEL:
        if ( SMTP_PMGR_DeleteEmailSeverityLevel(level) != SMTP_RETURN_SUCCESS)
        {
            CLI_LIB_PrintStr("Failed to delete logging sendmail level\r\n");
        }
        break;
        //end ADD
        
    default:
        break;
    }
#endif

    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Logging_Sendmail_SourceEmail
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "logging sendmail source-email"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Logging_Sendmail_SourceEmail(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SMTP==TRUE)
    UI8_T  email_addr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1] = {0};

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_LOGGING_SENDMAIL_SOURCEEMAIL:
	   strcpy((char *)email_addr,arg[0]);
       if ( SMTP_PMGR_SetSmtpSourceEmailAddr(email_addr) != SMTP_RETURN_SUCCESS)
       {
          CLI_LIB_PrintStr("Failed to set source-email address\r\n");
       }
       break;

       //ADD: daniel, 2008.08.07
    case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_LOGGING_SENDMAIL_SOURCEEMAIL:
       if (SMTP_PMGR_DeleteSmtpSourceEmailAddr() != SMTP_RETURN_SUCCESS)
       {
          CLI_LIB_PrintStr("Failed to delete source-email address\r\n");
       }
       break;
       //end ADD
       
    default:
       break;
    }
#endif

    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Logging_Sendmail
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show logging sendmail"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Logging_Sendmail(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SMTP==TRUE)
    UI32_T ip_addr = 0;
    UI32_T level = 0;
    UI32_T num = 0;
    UI32_T line_num = 0;
    UI8_T  email_addr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1] = {0};
    UI32_T status = 0;
    UI8_T  temp_addr[30]  = {0};
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE("SMTP Servers \r\n");
    PROCESS_MORE("-----------------------------------------------\r\n");
    while(SMTP_PMGR_GetNextSmtpServerIPAddr(&ip_addr) == SMTP_RETURN_SUCCESS)
    {
        num ++;
        L_INET_Ntoa(ip_addr, temp_addr);
        sprintf(buff, "  %lu. %s \r\n", num, temp_addr);
        PROCESS_MORE(buff);
    }
    PROCESS_MORE("\r\n");
    if(SMTP_PMGR_GetEmailSeverityLevel(&level)==SMTP_RETURN_SUCCESS)
    {
    	sprintf(buff, "SMTP Minimum Severity Level: %lu \r\n", level);
        PROCESS_MORE(buff);
    }
    PROCESS_MORE("\r\n");
    PROCESS_MORE("SMTP Destination E-mail Addresses\r\n");
    PROCESS_MORE("------------------------------------------------\r\n");
    num = 0;
    while(SMTP_PMGR_GetNextSmtpDestinationEmailAddr(email_addr)== SMTP_RETURN_SUCCESS)
    {
        num ++;
        sprintf(buff, "   %lu. %s \r\n", num, email_addr);
        PROCESS_MORE(buff);
    }
    PROCESS_MORE("\r\n");
    email_addr[0] = '\0';
    if(SMTP_PMGR_GetSmtpSourceEmailAddr(email_addr)==SMTP_RETURN_SUCCESS)
    {
        sprintf(buff, "SMTP Source E-mail Address:   %s \r\n", email_addr);
        PROCESS_MORE(buff);
    }
    PROCESS_MORE("\r\n");
    if( SMTP_PMGR_GetSmtpAdminStatus(&status)==SMTP_RETURN_SUCCESS)
    {
    	sprintf(buff, "SMTP Status:                %s \r\n", (status == SMTP_STATUS_ENABLE) ? "Enabled" : "Disabled");
        PROCESS_MORE(buff);
    }
#endif

   return CLI_NO_ERROR;
}

