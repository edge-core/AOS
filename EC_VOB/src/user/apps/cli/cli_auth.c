#include <string.h>
#include <stdio.h>
#include <assert.h>

/* system
 */
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_type.h"

/* common library
 */
#include "sysfun.h"
#include "l_md5.h"

/* driver
 */
#include "sys_time.h"

/* core
 */
#include "sys_mgr.h"
#include "userauth.h"
#include "userauth_pmgr.h"
#include "sshd_pmgr.h"
#include "sshd_mgr.h"
#include "mgmt_ip_flt.h"
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* CLI itself
 */
#include "cli_api.h"
#include "cli_def.h"
#include "cli_task.h"
#include "cli_lib.h"
#include "cli_pars.h"

static BOOL_T CLI_AUTH_CheckLoginPassword_LoginNoUsername(char sess_type, char *password, UI32_T *privilege);

BOOL_T CLI_AUTH_CheckLoginPassword(CLI_TASK_WorkingArea_T *ctrl_P, USERAUTH_AuthResult_T *auth_result_p)
{
    USERAUTH_Login_Method_T local_only_login_method;

    UI32_T                  TryTime = 0;

    char                    username[SYS_ADPT_MAX_USER_NAME_LEN + 1]         = {0};
    char                    password[SYS_ADPT_MAX_PASSWORD_LEN + 1]          = {0};

    char                    login_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN + 1]    = {0};
                           /*line passowrd*/
    UI32_T                  password_threshold = 0;
    UI32_T                  silent_time = 0;

    BOOL_T                  is_auth_forever    = FALSE;
    BOOL_T                  is_local_only      = FALSE;

    USERAUTH_Auth_Method_T  auth_method[5] = {0};   /*pttch*/
    UI32_T                  session_type = USERAUTH_SESSION_CONSOLE; /*maggie liu for arrangement userauth module*/
    UI32_T                  sess_id;
    L_INET_AddrIp_T         rem_ip_addr;

    /*----------------------------------------------------*/
    USERAUTH_PMGR_GetAuthMethod(auth_method);

    assert(ctrl_P != NULL);

    switch(ctrl_P -> sess_type)
    {
        case CLI_TYPE_UART:
        {
            SYS_MGR_Console_T  consoleCfg;

            memset(&consoleCfg, 0, sizeof(SYS_MGR_Console_T));
            SYS_PMGR_GetConsoleCfg(&consoleCfg);
            password_threshold = consoleCfg.password_threshold;
            silent_time   = consoleCfg.silent_time;

            USERAUTH_PMGR_GetConsolLoginMethod(&local_only_login_method);
            USERAUTH_PMGR_GetConsoleLoginPassword((UI8_T *)login_password);
            USERAUTH_PMGR_GetAuthMethod(auth_method);//pttch
            if ((auth_method[0] == USERAUTH_AUTH_LOCAL && auth_method[1] == USERAUTH_AUTH_NONE && auth_method[2] == USERAUTH_AUTH_NONE)
                || (auth_method[0] == USERAUTH_AUTH_NONE && auth_method[1] == USERAUTH_AUTH_NONE && auth_method[2] == USERAUTH_AUTH_NONE))
            {

                is_local_only = TRUE;
            }
            session_type = USERAUTH_SESSION_CONSOLE; /*maggie liu for arrangement userauth module*/
        }
        break;

        case CLI_TYPE_TELNET:
            session_type = USERAUTH_SESSION_TELNET; /*maggie liu for arrangement userauth module*/
#if (SYS_CPNT_SSH2 == TRUE)
            switch(ctrl_P -> CMenu.RemoteSessionType)
            {
                case CLI_TYPE_TELNET:
                {
                    SYS_MGR_Telnet_T telnetCfg;

                    memset(&telnetCfg, 0, sizeof(SYS_MGR_Telnet_T));
                    SYS_PMGR_GetTelnetCfg(&telnetCfg);
                    password_threshold = telnetCfg.password_threshold;
                    silent_time        = telnetCfg.silent_time;

                    USERAUTH_PMGR_GetTelnetLoginMethod(&local_only_login_method);
                    USERAUTH_PMGR_GetTelnetLoginPassword((UI8_T *)login_password);
                    USERAUTH_PMGR_GetAuthMethod(auth_method);
                    if ((auth_method[0] == USERAUTH_AUTH_LOCAL && auth_method[1] == USERAUTH_AUTH_NONE && auth_method[2] == USERAUTH_AUTH_NONE)
                        || (auth_method[0] == USERAUTH_AUTH_NONE && auth_method[1] == USERAUTH_AUTH_NONE && auth_method[2] == USERAUTH_AUTH_NONE))
                    {
                        is_local_only = TRUE;
                    }
                }
                break;


                case CLI_TYPE_SSH:

                /* Wait for SSH authentication result
                 */
                {
                    char c;
                    fd_set  rset;

                    FD_ZERO(&rset);
                    FD_SET(ctrl_P->socket, &rset);

                    /* Wait for user authentication
                     * do_exec_pty (security/sshv2/session.c)
                     */

                    /*printf ("CLI: wait for user authentication\r\n");*/

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                    SW_WATCHDOG_MGR_UnregisterMonitorThread(ctrl_P->monitor_id);
#endif

                    if (select(ctrl_P->socket + 1, &rset, NULL, NULL, NULL) < 0)
                    {
                        /*printf ("CLI: Select error for waitting SSH authen result\r\n");*/


#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                        SW_WATCHDOG_MGR_RegisterMonitorThread(ctrl_P->monitor_id, ctrl_P->cli_tid,
                            SYS_ADPT_CLI_SW_WATCHDOG_TIMER);
#endif

                        break;
                    }
                    if (recv(ctrl_P->socket, &c, 1, 0) < 0)
                    {
                        /*printf ("CLI: SSH authentication failed\r\n");*/

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                        SW_WATCHDOG_MGR_RegisterMonitorThread(ctrl_P->monitor_id, ctrl_P->cli_tid,
                            SYS_ADPT_CLI_SW_WATCHDOG_TIMER);
#endif

                        break;
                    }

                    /*printf ("CLI: SSH authentication succeeded\r\n");*/

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                    SW_WATCHDOG_MGR_RegisterMonitorThread(ctrl_P->monitor_id, ctrl_P->cli_tid,
                        SYS_ADPT_CLI_SW_WATCHDOG_TIMER);
#endif

                    if (TRUE == SSHD_MGR_GetSshConnectionAuthResult(CLI_TASK_GetMySessId() - CLI_TASK_MIN_TELNET_SESSION_ID + 1,
                                                                    auth_result_p))
                    {
                        UI32_T my_session_id = CLI_TASK_GetMySessId() - CLI_TASK_MIN_TELNET_SESSION_ID + 1;

                        ctrl_P->CMenu.AccessPrivilege = auth_result_p->privilege;
                        SSHD_MGR_GetSshConnectionUsername(my_session_id, (UI8_T*)ctrl_P->CMenu.UserName);
                        SSHD_MGR_GetSshConnectionPassword(my_session_id, ctrl_P->CMenu.Password, sizeof(ctrl_P->CMenu.Password));
                        return TRUE;
                    }
                    break;
                }

                default:
                    break;
            }
            break;
#else
        {
            SYS_MGR_Telnet_T telnetCfg;

            memset(&telnetCfg, 0, sizeof(SYS_MGR_Telnet_T));
            SYS_PMGR_GetTelnetCfg(&telnetCfg);
            password_threshold = telnetCfg.password_threshold;

            USERAUTH_PMGR_GetTelnetLoginMethod(&local_only_login_method);
            USERAUTH_PMGR_GetTelnetLoginPassword((UI8_T *)login_password);
            USERAUTH_PMGR_GetAuthMethod(auth_method);
            if ((auth_method[0] == USERAUTH_AUTH_LOCAL && auth_method[1] == USERAUTH_AUTH_NONE && auth_method[2] == USERAUTH_AUTH_NONE)
                || (auth_method[0] == USERAUTH_AUTH_NONE && auth_method[1] == USERAUTH_AUTH_NONE && auth_method[2] == USERAUTH_AUTH_NONE))
            {
                is_local_only = TRUE;
            }
        }

        break;
#endif /* #if (SYS_CPNT_SSH2 == TRUE) */

        case CLI_TYPE_PROVISION:
            auth_result_p->privilege = 0;
            auth_result_p->authen_by_whom = USERAUTH_AUTH_LOCAL;
            return TRUE;

        default:
            CLI_LIB_PrintStr("Unknown session type\r\n");
            return FALSE;
    }

    CLI_LIB_PrintStr("\r\n\r\n");
    CLI_LIB_PrintStr(SYS_ADPT_LOGIN_PROMPT_STRING);
    CLI_LIB_PrintStr("\r\n\r\n");
    is_auth_forever = (password_threshold == 0 ? TRUE : FALSE);

    for (TryTime=1; TryTime<=password_threshold || is_auth_forever; TryTime++)
    {
        memset(username, 0, sizeof(username));
        memset(password, 0, sizeof(password));
        if (is_local_only)
        {/*local only*/

            switch(local_only_login_method)
            {
                case USERAUTH_LOGIN_NO_LOGIN:
                    /*no matter console or vty*/
                    auth_result_p->privilege = 0;
                    auth_result_p->authen_by_whom = USERAUTH_AUTH_LOCAL;
                    ctrl_P->CMenu.AccessPrivilege = 0;
                    memset(ctrl_P->CMenu.UserName, 0, sizeof(ctrl_P->CMenu.UserName));
                    return TRUE;

                case USERAUTH_LOGIN_LOGIN:
                    if(strlen((char *)login_password) == 0)
                    {
                        auth_result_p->privilege = 0;
                        auth_result_p->authen_by_whom = USERAUTH_AUTH_LOCAL;
                        ctrl_P->CMenu.AccessPrivilege = 0;
                        memset(ctrl_P->CMenu.UserName, 0, sizeof(ctrl_P->CMenu.UserName));
                        return TRUE;
                    }
                    else
                    {
                        CLI_LIB_PrintStr("Password: ");
                        if(CLI_PARS_ReadLine(password, sizeof(password), TRUE, TRUE) == 3)
                        {
                            break;
                        }
                        CLI_LIB_PrintNullStr(1);
                        if(CLI_AUTH_CheckLoginPassword_LoginNoUsername(ctrl_P->sess_type, password, &auth_result_p->privilege))
                        {
                            ctrl_P->CMenu.AccessPrivilege = auth_result_p->privilege;
                            memset(ctrl_P->CMenu.UserName, 0, sizeof(ctrl_P->CMenu.UserName));
                            return TRUE;
                        }
                    }
                    break;

                case USERAUTH_LOGIN_LOGIN_LOCAL:
                    CLI_LIB_PrintStr("Username: ");
                    if(CLI_PARS_ReadLine(username, sizeof(username), TRUE, FALSE) == 3)
                    {
                        break;
                    }
                    CLI_LIB_PrintNullStr(1);

                    CLI_LIB_PrintStr("Password: ");
                    if(CLI_PARS_ReadLine(password, sizeof(password), TRUE, TRUE) == 3)
                    {
                       break;
                    }
                    CLI_LIB_PrintNullStr(1);

                    if(ctrl_P->sess_type == CLI_TYPE_UART)
                    {
                        sess_id = 0;
                        memset(&rem_ip_addr, 0, sizeof(rem_ip_addr));
                    }
                    else
                    {
                        sess_id = CLI_TASK_GetMySessId() - CLI_TASK_MIN_TELNET_SESSION_ID + 1;
                        memcpy(&rem_ip_addr, &ctrl_P->CMenu.TelnetIPAddress, sizeof(rem_ip_addr));
                    }

                    if (USERAUTH_PMGR_LoginAuthBySessionType(username,
                                                             password,
                                                             session_type,
                                                             sess_id,
                                                             &rem_ip_addr,
                                                             auth_result_p))
                    {
                        ctrl_P->CMenu.AccessPrivilege = auth_result_p->privilege;
                        strcpy((char *)ctrl_P->CMenu.UserName, (char *)username);
                        strncpy(ctrl_P->CMenu.Password, password, SYS_ADPT_MAX_PASSWORD_LEN);
                        ctrl_P->CMenu.Password[SYS_ADPT_MAX_PASSWORD_LEN] = '\0';
                        return TRUE;
                    }
                    break;

                default:
                    return FALSE;
            }
        }
        else /*except login local only*/
        {
            BOOL_T ret;

            CLI_LIB_PrintStr("Username: ");
            if(CLI_PARS_ReadLine(username, sizeof(username), TRUE, FALSE) == 3)
            {
                break;
            }
            CLI_LIB_PrintNullStr(1);

            CLI_LIB_PrintStr("Password: ");
            if(CLI_PARS_ReadLine(password, sizeof(password), TRUE, TRUE) == 3)
            {
                break;
            }

            CLI_LIB_PrintNullStr(1);

            if(ctrl_P->sess_type == CLI_TYPE_UART)
            {
                sess_id = 0;
                memset(&rem_ip_addr, 0, sizeof(rem_ip_addr));
            }
            else
            {
                sess_id = CLI_TASK_GetMySessId() - CLI_TASK_MIN_TELNET_SESSION_ID + 1;
                memcpy(&rem_ip_addr, &ctrl_P->CMenu.TelnetIPAddress, sizeof(rem_ip_addr));
            }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
            SW_WATCHDOG_MGR_UnregisterMonitorThread(ctrl_P->monitor_id);
#endif

            ret = USERAUTH_PMGR_LoginAuthBySessionType(username, password, session_type, sess_id,
                &rem_ip_addr, auth_result_p);

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
            SW_WATCHDOG_MGR_RegisterMonitorThread(ctrl_P->monitor_id, ctrl_P->cli_tid,
                SYS_ADPT_CLI_SW_WATCHDOG_TIMER);
#endif

            if (TRUE == ret)
            {
#if (SYS_CPNT_CLI_MULTI_PRIVILEGE_LEVEL == TRUE)
                ctrl_P->CMenu.AccessPrivilege = auth_result_p->privilege;
#else
                if(auth_result_p->privilege == USERAUTH_ADMIN_USER_PRIVILEGE)
                {
                    ctrl_P->CMenu.AccessPrivilege = USERAUTH_ADMIN_USER_PRIVILEGE;
                }
                else
                {
                    ctrl_P->CMenu.AccessPrivilege = USERAUTH_GUEST_USER_PRIVILEGE;
                }
#endif /* #if (SYS_CPNT_CLI_MULTI_PRIVILEGE_LEVEL == TRUE) */
                strcpy((char *)ctrl_P->CMenu.UserName, (char *)username);
                strncpy(ctrl_P->CMenu.Password, password, SYS_ADPT_MAX_PASSWORD_LEN);
                ctrl_P->CMenu.Password[SYS_ADPT_MAX_PASSWORD_LEN] = '\0';
                return TRUE;
            }
        }

        /*login fail*/
        if (TryTime >=  password_threshold && !is_auth_forever)
        {
            /*silent-timie*/
            if(ctrl_P -> sess_type  ==  CLI_TYPE_UART  &&  silent_time  !=  0)  /*0: disable*/
                SYSFUN_Sleep(silent_time*100);
            else if (ctrl_P->sess_type == CLI_TYPE_TELNET &&  silent_time != 0)
            {
                MGMT_IP_FLT_AddBlockCache(MGMT_IP_FLT_TELNET, &ctrl_P->CMenu.TelnetIPAddress, silent_time);
            }
            return FALSE;
        }
        else
            continue;
	}

    return FALSE;
}

static BOOL_T CLI_AUTH_CheckLoginPassword_LoginNoUsername(char sess_type, char *password, UI32_T *privilege)
{
   char get_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN + 1] = {0};
   char encrypted_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN + 1] = {0};

   switch(sess_type)
   {
   case CLI_TYPE_UART:
      USERAUTH_PMGR_GetConsoleLoginPassword((UI8_T *)get_password);
      break;

   case CLI_TYPE_TELNET:
      USERAUTH_PMGR_GetTelnetLoginPassword((UI8_T *)get_password);
      break;

   case CLI_TYPE_PROVISION:
   default:
      return FALSE;
   }

   memset(encrypted_password, 0, sizeof(encrypted_password));
   L_MD5_MDString((UI8_T *)encrypted_password, (UI8_T *)password, strlen(password));

   if(memcmp(get_password, encrypted_password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN) == 0)
   {
      *privilege = 0;
      return TRUE;
   }
   else
   {
      /*use the password of the superuser as the password when "login"; both UART and TELNET*/
      USERAUTH_LoginLocal_T login_user;

      memset(&login_user, 0, sizeof(USERAUTH_LoginLocal_T));
      strcpy((char *)login_user.username, USERAUTH_SUPERUSER_USERNAME_DEF);

      if(!USERAUTH_PMGR_GetLoginLocalUser(&login_user))
      {
         return FALSE;
      }
      else
      {
         if(memcmp(login_user.password, encrypted_password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN) != 0)
         {
            return FALSE;
         }
         else
         {
            *privilege = 16;
            return TRUE;
         }
      }
   }
   return FALSE;
}




BOOL_T CLI_AUTH_CheckEnablePassword(CLI_TASK_WorkingArea_T *ctrl_P, UI32_T *privilege)
{
   char  key_in[SYS_ADPT_MAX_PASSWORD_LEN + 1] = {0};
   UI8_T try_times;
   USERAUTH_Login_Method_T local_only_login_method;
   USERAUTH_Auth_Method_T  auth_method[5] = {0};
   UI32_T session_type = USERAUTH_SESSION_CONSOLE; /*maggie liu for arrangement userauth module*/
   UI32_T sess_id;
   L_INET_AddrIp_T  rem_ip_addr;

   assert(ctrl_P != NULL);

   for(try_times = 1; try_times <= 3; try_times++)
   {

      switch(ctrl_P -> sess_type)
      {
      case CLI_TYPE_UART:
         session_type = USERAUTH_SESSION_CONSOLE;
         sess_id = 0;
         memset(&rem_ip_addr, 0, sizeof(rem_ip_addr));
         USERAUTH_PMGR_GetConsolLoginMethod(&local_only_login_method);
         break;

      case CLI_TYPE_TELNET:
         session_type = USERAUTH_SESSION_TELNET;
         sess_id = CLI_TASK_GetMySessId() - CLI_TASK_MIN_TELNET_SESSION_ID + 1;
         memcpy(&rem_ip_addr, &ctrl_P->CMenu.TelnetIPAddress, sizeof(rem_ip_addr));
         USERAUTH_PMGR_GetTelnetLoginMethod(&local_only_login_method);
         break;

      case CLI_TYPE_PROVISION:
      default:
         return FALSE;
      }

      USERAUTH_PMGR_GetAuthMethod(auth_method);

      if (auth_method[0] != USERAUTH_AUTH_LOCAL || local_only_login_method != USERAUTH_LOGIN_NO_LOGIN)
      {
         memset(key_in, 0, SYS_ADPT_MAX_PASSWORD_LEN + 1);
         CLI_LIB_PrintStr("Password: ");

         if(CLI_PARS_ReadLine(key_in, SYS_ADPT_MAX_PASSWORD_LEN + 1, TRUE, TRUE) == 3)
         {
            break;
         }
      }

      CLI_LIB_PrintNullStr(1);

      switch(ctrl_P -> sess_type)
      {
      case CLI_TYPE_UART:
      case CLI_TYPE_TELNET:
         if (USERAUTH_PMGR_EnablePasswordAuth(ctrl_P->CMenu.UserName,
                                              key_in,
                                              session_type,
                                              sess_id,
                                              &rem_ip_addr,
                                              privilege) == TRUE)
         {
            return TRUE;
         }
         break;

      case CLI_TYPE_PROVISION:
         return TRUE;

      default:
         return FALSE;
      }
   }

   CLI_LIB_PrintStr("% Bad passwords\r\n");
   return FALSE;

}


//#else
#if 0


BOOL_T CLI_AUTH_CheckLoginPassword(CLI_TASK_WorkingArea_T *ctrl_P, UI32_T *got_user_privilege)
{
//   static BOOL_T CLI_AUTH_CheckLoginPassword_LoginLocal(UI8_T *username, UI8_T *password, UI32_T *privilege);
//   static BOOL_T CLI_AUTH_CheckLoginPassword_Radius(UI8_T *username, UI8_T *password, UI32_T *privilege);
//   static BOOL_T CLI_AUTH_CheckLoginPassword_LoginNoUsername(UI8_T sess_type, UI8_T *password, UI32_T *privilege);

   USERAUTH_Auth_Method_T  auth_rule;
   USERAUTH_Login_Method_T local_only_login_method;

   UI32_T                  TryTime = 0;

   UI8_T                   username[SYS_ADPT_MAX_USER_NAME_LEN + 1]         = {0};
   UI8_T                   password[SYS_ADPT_MAX_PASSWORD_LEN + 1]          = {0};

   UI8_T                  login_password[SYS_ADPT_MAX_PASSWORD_LEN + 1]    = {0};

   UI32_T                  password_threshold = 0;
   UI32_T                  uart_silent_time;

   BOOL_T                  is_auth_forever    = FALSE;

   /*----------------------------------------------------*/

   USERAUTH_PMGR_GetAuthMethod(&auth_rule);

   switch(ctrl_P -> sess_type)
   {
   case CLI_TYPE_UART:
      {
         SYS_MGR_Console_T  consoleCfg;

         memset(&consoleCfg, 0, sizeof(SYS_MGR_Console_T));
         SYS_PMGR_GetConsoleCfg(&consoleCfg);
         password_threshold = consoleCfg.password_threshold;
         uart_silent_time   = consoleCfg.silent_time;

         USERAUTH_PMGR_GetConsolLoginMethod(&local_only_login_method);
         USERAUTH_PMGR_GetConsoleLoginPassword(login_password);
      }
      break;

   case CLI_TYPE_TELNET:
      {
         SYS_MGR_Telnet_T telnetCfg;

         memset(&telnetCfg, 0, sizeof(SYS_MGR_Telnet_T));
         SYS_PMGR_GetTelnetCfg(&telnetCfg);
         password_threshold = telnetCfg.password_threshold;

         USERAUTH_PMGR_GetTelnetLoginMethod(&local_only_login_method);
         USERAUTH_PMGR_GetTelnetLoginPassword((UI8_T *)login_password);
      }
      break;

   case CLI_TYPE_PROVISION:
      *got_user_privilege = 0;
      return TRUE;

   default:
      CLI_LIB_PrintStr("Unknown session type\r\n");
      return FALSE;
   }

   CLI_LIB_PrintStr("\r\n\r\n");
   CLI_LIB_PrintStr(SYS_ADPT_LOGIN_PROMPT_STRING);
   CLI_LIB_PrintStr("\r\n\r\n");
   is_auth_forever = (password_threshold == 0 ? TRUE : FALSE);

   for (TryTime=1; TryTime<=password_threshold || is_auth_forever; TryTime++)
   {
      memset(username, 0, sizeof(username));
      memset(password, 0, sizeof(password));

      switch(auth_rule)
      {
      case USERAUTH_AUTH_LOCAL:
         switch(local_only_login_method)
         {
         case USERAUTH_LOGIN_NO_LOGIN:
            /*no matter console or vty*/
            *got_user_privilege     = 0;
            ctrl_P->CMenu.AccessPrivilege = 0;
            memset(ctrl_P->CMenu.UserName, 0, sizeof(ctrl_P->CMenu.UserName));
            return TRUE;

         case USERAUTH_LOGIN_LOGIN:
            if(strlen((char *)login_password) == 0)
            {
               *got_user_privilege     = 0;
               ctrl_P->CMenu.AccessPrivilege = 0;
               memset(ctrl_P->CMenu.UserName, 0, sizeof(ctrl_P->CMenu.UserName));
               return TRUE;
            }
            else
            {
               CLI_LIB_PrintStr("Password: ");
               if(CLI_PARS_ReadLine(password, sizeof(password), TRUE, TRUE) == 3)
               {
                  break;
               }

               CLI_LIB_PrintNullStr(1);

               if(CLI_AUTH_CheckLoginPassword_LoginNoUsername(ctrl_P->sess_type, password, got_user_privilege))
               {
                  ctrl_P->CMenu.AccessPrivilege = *got_user_privilege;
                  memset(ctrl_P->CMenu.UserName, 0, sizeof(ctrl_P->CMenu.UserName));
                  return TRUE;
               }
               else
               {
                  CLI_LIB_PrintStr("Login invalid\r\n\r\n");
               }
            }
            break;

         case USERAUTH_LOGIN_LOGIN_LOCAL:
            CLI_LIB_PrintStr("Username: ");
            if(CLI_PARS_ReadLine(username, sizeof(username), TRUE, FALSE) == 3)
            {
               break;
            }
            CLI_LIB_PrintNullStr(1);

            CLI_LIB_PrintStr("Password: ");
            if(CLI_PARS_ReadLine(password, sizeof(password), TRUE, TRUE) == 3)
            {
               break;
            }
            CLI_LIB_PrintNullStr(1);

            if(CLI_AUTH_CheckLoginPassword_LoginLocal(username, password, got_user_privilege))
            {
               ctrl_P->CMenu.AccessPrivilege = *got_user_privilege;
               strcpy(ctrl_P->CMenu.UserName, username);
               return TRUE;
            }
            else
            {
               CLI_LIB_PrintStr("Login invalid\r\n\r\n");
            }
            break;

         default:
            return FALSE;
         }
         break;

      case USERAUTH_AUTH_REMOTE_ONLY:
         CLI_LIB_PrintStr("Username: ");
         if(CLI_PARS_ReadLine(username, sizeof(username), TRUE, FALSE) == 3)
         {
            break;
         }
         CLI_LIB_PrintNullStr(1);

         CLI_LIB_PrintStr("Password: ");
         if(CLI_PARS_ReadLine(password, sizeof(password), TRUE, TRUE) == 3)
         {
            break;
         }
         CLI_LIB_PrintNullStr(1);

         if(CLI_AUTH_CheckLoginPassword_Radius(username, password, got_user_privilege))
         {
            ctrl_P->CMenu.AccessPrivilege = *got_user_privilege;
            strcpy(ctrl_P->CMenu.UserName, username);
            return TRUE;
         }
         else
         {
            CLI_LIB_PrintStr("Login invalid\r\n\r\n");
         }
         break;

      case USERAUTH_AUTH_REMOTE_THEN_LOCAL:
         CLI_LIB_PrintStr("Username: ");
         if(CLI_PARS_ReadLine(username, sizeof(username), TRUE, FALSE) == 3)
         {
            break;
         }
         CLI_LIB_PrintNullStr(1);

         CLI_LIB_PrintStr("Password: ");
         if(CLI_PARS_ReadLine(password, sizeof(password), TRUE, TRUE) == 3)
         {
            break;
         }
         CLI_LIB_PrintNullStr(1);

         if(CLI_AUTH_CheckLoginPassword_Radius(username, password, got_user_privilege))
         {
            ctrl_P->CMenu.AccessPrivilege = *got_user_privilege;
            strcpy(ctrl_P->CMenu.UserName, username);
            return TRUE;
         }
         else if(CLI_AUTH_CheckLoginPassword_LoginLocal(username, password, got_user_privilege))
         {
            ctrl_P->CMenu.AccessPrivilege = *got_user_privilege;
            strcpy(ctrl_P->CMenu.UserName, username);
            return TRUE;
         }
         else
         {
            CLI_LIB_PrintStr("Login invalid\r\n\r\n");
         }
         break;

      case USERAUTH_AUTH_LOCAL_THEN_REMOTE:
         CLI_LIB_PrintStr("Username: ");
         if(CLI_PARS_ReadLine(username, sizeof(username), TRUE, FALSE) == 3)
         {
            break;
         }
         CLI_LIB_PrintNullStr(1);

         CLI_LIB_PrintStr("Password: ");
         if(CLI_PARS_ReadLine(password, sizeof(password), TRUE, TRUE) == 3)
         {
            break;
         }
         CLI_LIB_PrintNullStr(1);

         if(CLI_AUTH_CheckLoginPassword_LoginLocal(username, password, got_user_privilege))
         {
            ctrl_P->CMenu.AccessPrivilege = *got_user_privilege;
            strcpy(ctrl_P->CMenu.UserName, username);
            return TRUE;
         }
         else if(CLI_AUTH_CheckLoginPassword_Radius(username, password, got_user_privilege))
         {
            ctrl_P->CMenu.AccessPrivilege = *got_user_privilege;
            strcpy(ctrl_P->CMenu.UserName, username);
            return TRUE;
         }
         else
         {
            CLI_LIB_PrintStr("Login invalid\r\n\r\n");
         }
         break;

      default:
         return FALSE;
      }

      //CLI_LIB_PrintStr("Login invalid\r\n\r\n");

      if (TryTime >=  password_threshold && !is_auth_forever)
      {
         /*silent-timie*/
         if(ctrl_P -> sess_type  ==  CLI_TYPE_UART  &&  uart_silent_time  !=  0)  /*0: disable*/
            SYSFUN_Sleep(uart_silent_time*100);

         return FALSE;
      }
      else
         continue;
   }
   return FALSE;
}


static BOOL_T CLI_AUTH_CheckLoginPassword_LoginLocal(UI8_T *username, UI8_T *password, UI32_T *privilege)
{
   USERAUTH_LoginLocal_T login_user;
   UI8_T encrypted_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1] = {0};

   memset(&login_user, 0, sizeof(login_user));
   strcpy(login_user.username, username);

   if(USERAUTH_PMGR_GetLoginLocalUser(&login_user))
   {
      /*
      if(strlen(login_user.password) == 0 && strlen(password) == 0)
      {
         *privilege = login_user.privilege;
         return TRUE;
      }
       */

      memset(encrypted_password, 0, sizeof(encrypted_password));
      L_MD5_MDString(encrypted_password, password, strlen(password));

      if(memcmp(login_user.password, encrypted_password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN) == 0)
      {
         *privilege = login_user.privilege;
         return TRUE;
      }
      else
      {
         return FALSE;
      }
   }

   return FALSE;
}

static BOOL_T CLI_AUTH_CheckLoginPassword_Radius(UI8_T *username, UI8_T *password, UI32_T *privilege)
{
   I32_T radius_privilege;

   if( RADIUS_Auth_Check(username, password, &radius_privilege) == 0) /*success*/
   {
      if(radius_privilege == AUTH_ADMINISTRATIVE)
         *privilege = 15;
      else if (radius_privilege == AUTH_LOGIN)
         *privilege = 0;
      else
         return FALSE;

      return TRUE;
   }

   return FALSE;
}


static BOOL_T CLI_AUTH_CheckLoginPassword_LoginNoUsername(UI8_T sess_type, UI8_T *password, UI32_T *privilege)
{
   UI8_T get_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN + 1] = {0};
   UI8_T encrypted_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN + 1] = {0};

   switch(sess_type)
   {
   case CLI_TYPE_UART:
      USERAUTH_PMGR_GetConsoleLoginPassword(get_password);
      break;

   case CLI_TYPE_TELNET:
      USERAUTH_PMGR_GetTelnetLoginPassword(get_password);
      break;

   case CLI_TYPE_PROVISION:
   default:
      return FALSE;
   }

   memset(encrypted_password, 0, sizeof(encrypted_password));
   L_MD5_MDString(encrypted_password, password, strlen(password));

   if(memcmp(get_password, encrypted_password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN) == 0)
   {
      *privilege = 0;
      return TRUE;
   }
   else
   {
      /*use the password of the superuser as the password when "login"; both UART and TELNET*/
      USERAUTH_LoginLocal_T login_user;

      memset(&login_user, 0, sizeof(USERAUTH_LoginLocal_T));
      strcpy(login_user.username, USERAUTH_SUPERUSER_USERNAME_DEF);

      if(!USERAUTH_PMGR_GetLoginLocalUser(&login_user))
         return FALSE;
      else
      {
         if(memcmp(login_user.password, encrypted_password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN) != 0)
            return FALSE;
         else
         {
            *privilege = 16;
            return TRUE;
         }
      }
   }

   return FALSE;
}




BOOL_T CLI_AUTH_CheckEnablePassword(CLI_TASK_WorkingArea_T *ctrl_P, UI32_T privilege)
{
   static BOOL_T CLI_AUTH_IsThereAnyEnablePassword(void);

   USERAUTH_Privilege_Password_T privilege_password;
   UI8_T key_in[SYS_ADPT_MAX_PASSWORD_LEN + 1] = {0};
   UI8_T encrypted_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1] = {0};
   UI8_T try_times;

   memset(&privilege_password, 0, sizeof(USERAUTH_Privilege_Password_T));
   privilege_password.privilege = privilege;

   switch(ctrl_P -> sess_type)
   {
   case CLI_TYPE_UART:
      {
         /* 1. if no any enable password is set, return TRUE whatever*/
         /* 2. if currently the <no login> is enabled, return TRUE whatever*/

         USERAUTH_Login_Method_T local_only_login_method;
         USERAUTH_Auth_Method_T  auth_rule;

         USERAUTH_PMGR_GetConsolLoginMethod(&local_only_login_method);
         USERAUTH_PMGR_GetAuthMethod(&auth_rule);

          if( !CLI_AUTH_IsThereAnyEnablePassword()  ||
                ( auth_rule == USERAUTH_AUTH_LOCAL_ONLY && local_only_login_method == USERAUTH_LOGIN_NO_LOGIN))
          {
             return TRUE;
          }
       }
      break;

   case CLI_TYPE_TELNET:
      ;
      break;

   case CLI_TYPE_PROVISION:
      return TRUE;

   default:
      return FALSE;
   }

   if(!USERAUTH_PMGR_GetPrivilegePassword(&privilege_password))
   {
      CLI_LIB_PrintStr("% No password set\r\n");
      return FALSE;
   }
   else
   {
      for(try_times = 1; try_times <= 3; try_times++)
      {
         memset(key_in, 0, SYS_ADPT_MAX_PASSWORD_LEN + 1);
         memset(encrypted_password, 0, SYS_ADPT_MAX_PASSWORD_LEN + 1);

         CLI_LIB_PrintStr("Password: ");

         if(CLI_PARS_ReadLine(key_in, SYS_ADPT_MAX_PASSWORD_LEN + 1, TRUE, TRUE) == 3)
         {
            break;
         }
         CLI_LIB_PrintNullStr(1);

         memset(encrypted_password, 0, sizeof(encrypted_password));
         L_MD5_MDString(encrypted_password, key_in, strlen(key_in));

         if(memcmp(encrypted_password, privilege_password.password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN) == 0)
            return TRUE;
      }

      CLI_LIB_PrintStr("% Bad passwords\r\n");
      return FALSE;
   }

   return FALSE;
}



static BOOL_T CLI_AUTH_IsThereAnyEnablePassword(void)
{
   UI32_T                        privilege;
   USERAUTH_Privilege_Password_T privilege_password;
   BOOL_T                        is_got = TRUE;;

   for(privilege = 1; privilege<= SYS_ADPT_MAX_LOGIN_PRIVILEGE; privilege++)
   {
      memset(&privilege_password, 0, sizeof(USERAUTH_Privilege_Password_T));
      privilege_password.privilege = privilege;

      if(USERAUTH_PMGR_GetPrivilegePassword(&privilege_password))
      {
         is_got = TRUE;
         break;
      }
   }

   if(is_got)
      return TRUE;
   else
      return FALSE;
}


#endif
