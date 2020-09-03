#include <stdio.h>
#include <string.h>
#include "libtacacs.h"
#include "tacacs_mgr.h"
#include "sys_module.h"
#include "l_mm.h"
#include "sys_time.h"
#include "tac_account_c.h"

/*** REPLACE WITH YOUR PARAMETERS FIRST ! ***/
#define TAC_SERVER   "10.1.1.1"
#define TAC_TIMEOUT   5
#define TAC_KEY      "Cisco"
#define TAC_PORT      0
#define USER         "username"

#define TIME_1900_TO_1970_SECS       0x83aa7e80     /* 1970 - 1900 in seconds */
#define TIME_1900_TO_2001_SECS       0xBDFA4700     /* Seconds between 1900 and 2001*/

/* NAMING CONSTANT DECLARATIONS
 */
/* define naming constant TAC_ACCT_C_DEBUG_MODE
 * to build tac_account.c with DEBUG version
 * And let following macros print debug messages
 */
#define TAC_ACCT_C_DEBUG_MODE

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef TAC_ACCT_C_DEBUG_MODE
#include "backdoor_mgr.h"

    #define TAC_ACCT_C_TRACE(fmt, ...)                (TACACS_LOG(TACACS_MGR_GetDebugFlag(TACACS_ACCT_C), fmt, ##__VA_ARGS__))

#else

    #define TAC_ACCT_C_TRACE(fmt, ...)                ((void)0)

#endif /* TAC_AUTHEN_C_DEBUG_MODE */


#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
BOOL_T tacacs_acc_main(
    UI32_T server_ip,
    UI8_T *secret,
    UI32_T server_port,
    UI32_T retransmit,
    UI32_T timeout,
    int flag,
    TACACS_AccUserInfo_T *user_info,
    UI32_T current_sys_time)
{
    /* The maximum length of an attribute-value string is 256 characters.
     * But we doesn't use so many memory for task id, start time, stop time, elapsed time, and port.
     */
    enum {MAX_LENGTH_OF_AN_AVPAIR_STRING = 30};

    int status;
    int method;
    int connect_status;
    int retries = 0;
    char *avpair[TACACS_LIB_MAX_NBR_OF_AV_PAIRS];
    char *data;
    char *msg;
    UI32_T time;
    UI32_T offset = TIME_1900_TO_2001_SECS - TIME_1900_TO_1970_SECS;
    struct session  session_buf;
    char task_id_str[MAX_LENGTH_OF_AN_AVPAIR_STRING]     = {0};
    char start_time_str[MAX_LENGTH_OF_AN_AVPAIR_STRING]  = {0};
    char stop_time_str[MAX_LENGTH_OF_AN_AVPAIR_STRING]   = {0};
    char elapsed_time_str[MAX_LENGTH_OF_AN_AVPAIR_STRING]= {0};
    char timezone[MAXSIZE_sysTimeZoneName+1]             = {0};
    char timezone_str[MAXSIZE_sysTimeZoneName+9+1]       = {0};
    char port[TACACS_TYPE_MAX_LEN_OF_NAS_PORT + 1];
    char rem_addr[TACACS_TYPE_MAX_LEN_OF_REM_ADDR + 1];

    if ((data = (char *)L_MM_Malloc(sizeof(char)*TACACS_LIB_MAX_LEN_OF_PACKET_BODY, L_MM_USER_ID2(SYS_MODULE_TACACS, TACACS_TYPE_TRACE_ID_TACACS_ACCOUNT_C_MAIN))) == NULL)
        return FALSE;
    if ((msg = (char *)L_MM_Malloc(sizeof(char)*TACACS_LIB_MAX_LEN_OF_PACKET_BODY, L_MM_USER_ID2(SYS_MODULE_TACACS, TACACS_TYPE_TRACE_ID_TACACS_ACCOUNT_C_MAIN))) == NULL)
    {
        L_MM_Free(data);
        return FALSE;
    }

    /* task id */
    sprintf(task_id_str,"task_id=%ld",user_info->task_id);
    avpair[0] = task_id_str;

    /* timezone */
    SYS_TIME_GetTimeZoneNameByStr(timezone);
    sprintf(timezone_str,"timezone=%s",timezone);
    avpair[1] = timezone_str;

    /* start time */
    time = user_info->session_start_time + offset;
    sprintf(start_time_str,"start_time=%ld",time);
    avpair[2]=start_time_str;

    /* elapsed time */
    time = current_sys_time-user_info->session_start_time;
    sprintf(elapsed_time_str,"elapsed_time=%ld",time);
    avpair[3]=elapsed_time_str;

    /* stop time */
    if(flag == TAC_PLUS_ACCT_FLAG_STOP)
    {
        time = current_sys_time + offset;
        sprintf(stop_time_str,"stop_time=%ld",time);
        avpair[4]=stop_time_str;
    }
    else
    {
        avpair[4]=NULL;
    }
    avpair[5]=NULL;
    avpair[6]=NULL;

    if(user_info->auth_by_whom == AAA_AUTHEN_BY_RADIUS)
    {
        method = TAC_PLUS_AUTHEN_METH_RADIUS;
    }
    else if(user_info->auth_by_whom == AAA_AUTHEN_BY_TACACS_PLUS)
    {
        method = TAC_PLUS_AUTHEN_METH_TACACSPLUS;
    }
    else if(user_info->auth_by_whom == AAA_AUTHEN_BY_LOCAL)
    {
        method = TAC_PLUS_AUTHEN_METH_LOCAL;
    }
    else
    {
        method = TAC_PLUS_AUTHEN_METH_NOT_SET;
    }

    if (user_info->client_type == AAA_CLIENT_TYPE_EXEC)
    {
        TAC_UTILS_GetNasPort(
            ((user_info->ifindex == 0) ? TACACS_SESS_TYPE_CONSOLE : TACACS_SESS_TYPE_TELNET),
            ((user_info->ifindex == 0) ? 0 : user_info->ifindex-1),
            port,
            sizeof(port));

        TAC_UTILS_GetRemAddr(
            ((user_info->ifindex == 0) ? TACACS_SESS_TYPE_CONSOLE : TACACS_SESS_TYPE_TELNET),
            &user_info->rem_ip_addr,
            rem_addr,
            sizeof(rem_addr));
    }
    else
    {
        TAC_ACCT_C_TRACE("Wrong client_type=%d", user_info->client_type);
    }

    while(1)
    {
        /* initiate connection */
        connect_status = tac_connect_nonblocking(server_ip, secret, server_port, timeout, &session_buf);
        if (connect_status == -1)       /*tac_connect_nonb timeout*/
        {
            if (retries++ >= retransmit)
            {
                TAC_ACCT_C_TRACE("TACACS/ACCT: Connection error");

                L_MM_Free(data);
                L_MM_Free(msg);
                return FALSE;
            }
            continue;
        }
        else if (connect_status == 0) /*tac_connect_nonb failure*/
        {
            TAC_ACCT_C_TRACE("TACACS/ACCT: Connection error");

            L_MM_Free(data);
            L_MM_Free(msg);
            return FALSE;
        }

        if (1 != tac_account_send_request(&session_buf,
                                          flag,
                                          method,/*TAC_PLUS_AUTHEN_METH_TACACSPLUS,*/
                                          user_info->auth_privilege,/*TAC_PLUS_PRIV_LVL_MIN,*/
                                          TAC_PLUS_AUTHEN_TYPE_ASCII,
                                          TAC_PLUS_AUTHEN_SVC_LOGIN,
                                          user_info->user_name,
                                          port,
                                          rem_addr,
                                          avpair,
                                          secret))
        {
            TAC_ACCT_C_TRACE("TACACS/ACCT: Fail to send request");
            tac_close(&session_buf);
            if (retries++ >= retransmit)
            {
                L_MM_Free(data);
                L_MM_Free(msg);
                return FALSE;
            }
            continue;
        }

        status = tac_account_get_reply(&session_buf,msg,data,secret);
        tac_close(&session_buf);
        break;
    }

    L_MM_Free(data);
    L_MM_Free(msg);
    if(status == 1)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)
BOOL_T TAC_ACCOUNT_C_AcctCmdMain( UI32_T server_ip,
                                  UI8_T *secret,
                                  UI32_T server_port,
                                  UI32_T timeout,
                                  UI32_T retransmit,
                                  int    flag,
                                  const TPACC_AccCommandmdMessage_T *cmd_entry_p,
                                  UI32_T current_sys_time)
{
    /* The maximum length of an attribute-value string is 256 characters.
     * But we doesn't use so many memory for task id, service, privilege and port.
     */
    enum {MAX_LENGTH_OF_AN_AVPAIR_STRING = 30};

    int connect_status;
    int retries = 0;
    int method;
    int status;
    char *avpair[TACACS_LIB_MAX_NBR_OF_AV_PAIRS];
    char *data;
    char *msg;
    struct session  session_buf;
    char task_id_str[MAX_LENGTH_OF_AN_AVPAIR_STRING]  = {0};
    char service_str[MAX_LENGTH_OF_AN_AVPAIR_STRING]  = {0};
    char priv_lvl_str[MAX_LENGTH_OF_AN_AVPAIR_STRING] = {0};
    char cmd_str[SYS_ADPT_CLI_MAX_BUFSIZE + 4 +1]   = {0}; /*MAX command length + "cmd=" + '\0' */
    char port[TACACS_TYPE_MAX_LEN_OF_NAS_PORT + 1];
    char rem_addr[TACACS_TYPE_MAX_LEN_OF_REM_ADDR + 1];

   if ((data = (char *)L_MM_Malloc(TACACS_LIB_MAX_LEN_OF_PACKET_BODY,
                                   L_MM_USER_ID2(SYS_MODULE_TACACS, TACACS_TYPE_TRACE_ID_TACACS_ACCOUNT_C_ACCTCMDMAIN_D))) == NULL)
       return FALSE;
   if ((msg = (char *)L_MM_Malloc(TACACS_LIB_MAX_LEN_OF_PACKET_BODY,
                                   L_MM_USER_ID2(SYS_MODULE_TACACS, TACACS_TYPE_TRACE_ID_TACACS_ACCOUNT_C_ACCTCMDMAIN_M))) == NULL)
   {
       L_MM_Free(data);
       return FALSE;
   }

   /* task id */
   sprintf(task_id_str,"task_id=%ld", cmd_entry_p->serial_number);
   avpair[0] = task_id_str;

   /* service=shell */
   sprintf(service_str,"service=shell");
   avpair[1] = service_str;

   /* priv-lvl */
   sprintf(priv_lvl_str,"priv-lvl=%ld", cmd_entry_p->command_privilege);
   avpair[2] = priv_lvl_str;

   /* cmd */
   sprintf(cmd_str,"cmd=%s", cmd_entry_p->command);
   avpair[3]=cmd_str;

   avpair[4]=NULL; /* End of the AV-Pair List */

   if(cmd_entry_p->user_auth_by_whom == AAA_AUTHEN_BY_RADIUS)
   {
       method = TAC_PLUS_AUTHEN_METH_RADIUS;
   }
   else if(cmd_entry_p->user_auth_by_whom == AAA_AUTHEN_BY_TACACS_PLUS)
   {
       method = TAC_PLUS_AUTHEN_METH_TACACSPLUS;
   }
   else if(cmd_entry_p->user_auth_by_whom == AAA_AUTHEN_BY_LOCAL)
   {
       method = TAC_PLUS_AUTHEN_METH_LOCAL;
   }
   else
   {
       method = TAC_PLUS_AUTHEN_METH_NOT_SET;
   }

    TAC_UTILS_GetNasPort(
        ((cmd_entry_p->ifindex == 0)?TACACS_SESS_TYPE_CONSOLE:TACACS_SESS_TYPE_TELNET),
        ((cmd_entry_p->ifindex == 0)?0:cmd_entry_p->ifindex-1),
        port,
        sizeof(port));


    TAC_UTILS_GetRemAddr(
        ((cmd_entry_p->ifindex == 0)?TACACS_SESS_TYPE_CONSOLE:TACACS_SESS_TYPE_TELNET),
        &cmd_entry_p->rem_ip_addr,
        rem_addr,
        sizeof(rem_addr));

    while (1)
    {
        /* initiate connection */
        connect_status = tac_connect_nonblocking(server_ip,secret,server_port,timeout,&session_buf);
        if (connect_status == -1)       /*tac_connect_nonb timeout*/
        {
            if (retries++ >= retransmit)
            {
                TAC_ACCT_C_TRACE("TACACS/ACCT/CMD: Connection error");

                L_MM_Free(data);
                L_MM_Free(msg);
                return FALSE;
            }
            continue;
        }
        else if (connect_status == 0) /*tac_connect_nonb failure*/
        {
            TAC_ACCT_C_TRACE("TACACS/ACCT/CMD: Connection error");

            L_MM_Free(data);
            L_MM_Free(msg);
            return FALSE;
        }

        if (1 != tac_account_send_request(&session_buf,
                                          flag,
                                          method,                             /*TAC_PLUS_AUTHEN_METH_TACACSPLUS,*/
                                          cmd_entry_p->user_auth_privilege,   /*TAC_PLUS_PRIV_LVL_MIN,*/
                                          TAC_PLUS_AUTHEN_TYPE_ASCII,
                                          TAC_PLUS_AUTHEN_SVC_LOGIN,
                                          cmd_entry_p->user_name,             /*USER,*/
                                          port,                               /*PORT,*/
                                          rem_addr,
                                          (char**)avpair,
                                          secret))
        {
            TAC_ACCT_C_TRACE("TACACS/ACCT/CMD: Fail to send request");
            tac_close(&session_buf);
            if (retries++ >= retransmit)
            {
                L_MM_Free(data);
                L_MM_Free(msg);
                return FALSE;
            }
            continue;
        }

        status = tac_account_get_reply(&session_buf,msg,data,secret);
        tac_close(&session_buf);
        break;
    }

   L_MM_Free(data);
   L_MM_Free(msg);
   if(status == 1)
   {
       return TRUE;
   }
   else
   {
       return FALSE;
   }
}
#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)*/
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */

