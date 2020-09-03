/* ----------------------------------------------------------------------
 * FILE NAME: cgi\cg_auth.c

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Module for Fileless CGI.
 * This is the program for authorisation check.

 * CLASSES AND FUNCTIONS:

 * Check authorisation level:

 * cgi_check_auth: Check authorisation level.
 * cgi_check_pass: Check whether user name and password are valid.

 * Error replies:

 * HISTORY:
 * 1998-08-12 (Wed): Created by Daniel K. Chung, from "cg_main.c".

 * COPYRIGHT (C) Accton Technology Corporation, 1998.
 * ---------------------------------------------------------------------- */
#include "radius_mgr.h"
#include "cgi.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "trap_event.h"

#include "userauth_pmgr.h"
#include "l_md5.h"
#include "sys_time.h"
/* ----------------------------------------------------------------------
 * Constants.
 * ---------------------------------------------------------------------- */

#define CGI_ENCRIPT

#define MAX_HEADER_LEN  2048        /* this should probably be same as TCP buf size */
#define MAX_URI_LEN     256     /* maximum length a URI */
#define MAX_RELOAD_PATH_TEXT_LEN      256         /* maximum length of a reload path text */
#define MAX_RELOAD_PATH_LEN       64         /* maximum length of a reload path */
#ifndef HTTP_ERROR
#define HTTP_ERROR (-1)
#endif
#define HTTP_OK 0

static CGI_AUTH_LoginLocal_T  cgi_auth_login_local[SYS_ADPT_HTTP_RADIUS_CACHE_SIZE];
//static UI32_T IPCompare(L_INET_AddrIp_T *ip_addr_a, L_INET_AddrIp_T *ip_addr_b);
/* ----------------------------------------------------------------------
 * Forward declarations.
 * ---------------------------------------------------------------------- */
static int cgi_get_reload_path_text (
    envcfg_t *envcfg,
    char *auth_reload_path_text,
    char *auth_reload_path,
    const char *path
);

static void cgi_set_related_path(
    char *auth_reload_path_text,
    const char *path
);

/* ----------------------------------------------------------------------
 * cgi_check_auth: Check authorisation level.

 * Same as when an attempt is being made to access a secured directory.
 * Get encrypted authorization from env variable and decode.
 * Then compare with registerded user/password.

 * Return:  [Huffman* 06/12/2002]
 * -1: username/password UNauthorised;
 * 0-15: user privilege (0:guest, 15:admin);
 * ---------------------------------------------------------------------- */
int cgi_check_auth(HTTP_Connection_T *http_connection, HTTP_Request_T *req, envcfg_t *envQuery)
{
    CONNECTION_STATE_T  connection_status;
    char auth_reload_path_text[MAX_RELOAD_PATH_TEXT_LEN] = {0};
    char login_path_text[MAX_RELOAD_PATH_LEN] = {0};
    char index_path_text[MAX_RELOAD_PATH_LEN] = {0};
    char auth_level_str[3] = {0};
    char *auth_level, *username, *password;
    char *cookie_p;

    // FIXME: 1. Use "PATH_INFO" instead of "URI"
    //        2. MAY create a 'public' directory that dont need to authenticate.
    //        3. Create a directory 'public' to put the files that no need to do authentication,
    //           and this directory shall not be able to add to alias.
    const char *path = get_env (req->envcfg, "URI");

    BOOL_T bHaveAuth = FALSE;
    int nAuth;
    L_INET_AddrIp_T           rem_ip_addr;
    HTTP_Session_T sess = {{0}};
    USERAUTH_AuthResult_T     auth_result;
#if (SYS_CPNT_AUTHORIZATION == TRUE)
    AAA_AuthorRequest_T       author_request;
    AAA_AuthorReply_T         author_reply;
    BOOL_T                    author_result;
#endif /*#if (SYS_CPNT_AUTHORIZATION == TRUE)*/

    if (NULL == path)
    {
        set_env(req->envcfg, "AUTH_LEVEL", "UnAuth");
        cgi_SendHeader(http_connection, (int) req->fd, -1, req->envcfg);

        if (cgi_get_reload_path_text (req->envcfg, auth_reload_path_text, login_path_text, "") == 0)
        {
            cgi_SendText(http_connection, (int) req->fd, (const char *)auth_reload_path_text);
        }

        cgi_response_end(http_connection);
        return (-1);
    }

    // FIXME: DONT DO THAT. Add config for 'no-auth' path(s)
    if (strncmp(path, "/api/", 5) == 0)
    {
        return 99;
    }
    else
    {
         http_connection->done = 1;
         return (-1);
    }
}

/* ----------------------------------------------------------------------
 * cgi_set_related_path: set related path of current URL
 * Input:  auth_reload_path_text, path
 * Output: auth_reload_path_text
 * Return: none
 * Note:
 *      [example]*path = "\", strcat path "."
 *               *path = "\home\", strcat path ".."
 *               *path = "\home\316\", strcat path "../.."
 * ---------------------------------------------------------------------- */
static void cgi_set_related_path(char *auth_reload_path_text, const char *path)
{
    int  path_slash = 0, i;

    while (0 != *path)
    {
        if ('/' == *path)
        {
            path_slash ++;
        }
        path ++;
    }

    if (path_slash <= 1)
    {
        strncat(auth_reload_path_text, ".", MAX_RELOAD_PATH_TEXT_LEN-strlen(auth_reload_path_text));
    }
    else
    {
        for(i=0; i<(path_slash-1); i++)
        {
            if (i > 0)
            {
                strncat(auth_reload_path_text, "/", MAX_RELOAD_PATH_TEXT_LEN-strlen(auth_reload_path_text));
            }
            strncat(auth_reload_path_text, "..", MAX_RELOAD_PATH_TEXT_LEN-strlen(auth_reload_path_text));
        }
    }
}

/* ----------------------------------------------------------------------
 * cgi_get_reload_path_text: get reload URL path text if connection unauthorized
 * Input:  envcfg
 * Output: auth_reload_path_text
 * Return: failed -1, Success 0
 * Note: Brian, 04-18-2008
 * ---------------------------------------------------------------------- */
static int cgi_get_reload_path_text (envcfg_t *envcfg, char *auth_reload_path_text, char *auth_reload_path, const char *path)
{
    /* for cluster operate
     */
    if ((get_env(envcfg, "REMOTE_CLUSTER_ID") != NULL))
    {
        strcpy(auth_reload_path_text, "<html><body><script>window.location.href='");
        cgi_set_related_path(auth_reload_path_text, path);
        strncat(auth_reload_path_text, auth_reload_path, MAX_RELOAD_PATH_TEXT_LEN-strlen(auth_reload_path_text));
        strncat(auth_reload_path_text, "';</script></body></html>", MAX_RELOAD_PATH_TEXT_LEN-strlen(auth_reload_path_text));
        return (0);
    }

    strcpy(auth_reload_path_text, "<html><body><script>if (window.opener != null){window.opener.location.href='");
    cgi_set_related_path(auth_reload_path_text, path);
    strncat(auth_reload_path_text, auth_reload_path, MAX_RELOAD_PATH_TEXT_LEN-strlen(auth_reload_path_text));
    strncat(auth_reload_path_text, "'; window.close();} else{window.top.location.href='", MAX_RELOAD_PATH_TEXT_LEN-strlen(auth_reload_path_text));
    cgi_set_related_path(auth_reload_path_text, path);
    strncat(auth_reload_path_text, auth_reload_path, MAX_RELOAD_PATH_TEXT_LEN-strlen(auth_reload_path_text));
    strncat(auth_reload_path_text, "';}</script></body></html>", MAX_RELOAD_PATH_TEXT_LEN-strlen(auth_reload_path_text));
    return (0);
}

#if 0 //not used
static UI32_T IPCompare(L_INET_AddrIp_T *ip_addr_a, L_INET_AddrIp_T *ip_addr_b)
{
    static UI8_T v4_prefix[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF};
    static int v4_prefix_len = sizeof(v4_prefix);
    UI8_T  temp_addr_a[SYS_ADPT_IPV6_ADDR_LEN]={0};
    UI8_T  temp_addr_b[SYS_ADPT_IPV6_ADDR_LEN]={0};

    /* convert to ipv4-map-ipv6
     */
    if ( (L_INET_ADDR_TYPE_IPV4 == ip_addr_a->type)
        || (L_INET_ADDR_TYPE_IPV4Z == ip_addr_a->type) )
    {
        memset( temp_addr_a, 0, SYS_ADPT_IPV6_ADDR_LEN);
        memcpy( temp_addr_a, v4_prefix, v4_prefix_len);
        memcpy( &temp_addr_a[v4_prefix_len], &ip_addr_a->addr, SYS_ADPT_IPV4_ADDR_LEN);
    }
    else
    {
        memcpy( temp_addr_a, &ip_addr_a->addr, SYS_ADPT_IPV6_ADDR_LEN);
    }

    if ( (L_INET_ADDR_TYPE_IPV4 == ip_addr_b->type)
        ||(L_INET_ADDR_TYPE_IPV4Z == ip_addr_b->type) )
    {
        memset( temp_addr_b, 0, SYS_ADPT_IPV6_ADDR_LEN);
        memcpy( temp_addr_b, v4_prefix, v4_prefix_len);
        memcpy( &temp_addr_b[v4_prefix_len], &ip_addr_b->addr, SYS_ADPT_IPV4_ADDR_LEN);
    }
    else
    {
        memcpy( temp_addr_b, &ip_addr_b->addr, SYS_ADPT_IPV6_ADDR_LEN);
    }

    return memcmp(temp_addr_a, temp_addr_b, SYS_ADPT_IPV6_ADDR_LEN);
}
#endif

/*---------------------------------------------------------------------------------
 * cgi_check_pass: Check whether user name and password are valid.
 * Return:  [Huffman* 06/12/2002]
 * -1: username/password UNauthorised;
 * 0-15: user privilege (0:guest, 15:admin);
----------------------------------------------------------------------------------*/
int cgi_check_pass (
    CONNECTION_STATE_T connection_status,
    const char * username,
    const char * password,
    L_INET_AddrIp_T *rem_ip_addr,
    USERAUTH_AuthResult_T *auth_result_p)
{
    UI32_T  temp_privilege, privilege = -1, i;
    UI8_T   encode_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN + 1] = {0};

    //
    // FIXME: Why use SYS_CPNT_TACACS in here? All this function use this constant
    //        should have issue.
    //
#if (SYS_CPNT_TACACS == TRUE)
    USERAUTH_Auth_Method_T  auth_rule[USERAUTH_NUMBER_Of_AUTH_METHOD];
    USERAUTH_SessionType_T  sess_type;
#else
    UI32_T  radius_privilege;
    BOOL_T  is_username_exist = FALSE;
    USERAUTH_Auth_Method_T  auth_rule;
#endif
    USERAUTH_LoginLocal_T     User;
    //USERAUTH_Auth_Method_T    auth_by_whom = USERAUTH_AUTH_NONE;
    //HTTP_Session_T web_user_connection_info;

    memset(&User, 0 , sizeof(USERAUTH_LoginLocal_T));

    if ((username == NULL) || (password == NULL))
    {
        return(-1);
    }

#ifdef CGI_ENCRIPT
    L_MD5_MDString(encode_password, (UI8_T *)password, strlen(password));
#else
    strcpy(encode_password, password);
#endif

#if (SYS_CPNT_TACACS == TRUE)
    switch (connection_status)
    {
        case HTTP_CONNECTION:
            sess_type = USERAUTH_SESSION_HTTP;
            break;

        case HTTPS_CONNECTION:
            sess_type = USERAUTH_SESSION_HTTPS;
            break;

        case UNKNOW_CONNECTION:
        default:
            return(-1);
    }

    if (!USERAUTH_PMGR_GetAuthMethod(auth_rule))
    {
      return (-1);
    }

    //
    // FIXME: Does we still use this?
    //
    /* compare username & password with cgi_cache */
    if (((auth_rule[0] == USERAUTH_AUTH_RADIUS) || (auth_rule[0] == USERAUTH_AUTH_TACACS))
        || ((auth_rule[0] == USERAUTH_AUTH_LOCAL) && ((auth_rule[1] == USERAUTH_AUTH_RADIUS) ||(auth_rule[1] == USERAUTH_AUTH_TACACS)))
        || ((auth_rule[1] == USERAUTH_AUTH_LOCAL) && ((auth_rule[2] == USERAUTH_AUTH_RADIUS) || (auth_rule[2] == USERAUTH_AUTH_TACACS))))
    {
        for (i = 0; i< USERAUTH_NUMBER_Of_AUTH_METHOD;i++)
        {
            if ((auth_rule[i] ==  USERAUTH_AUTH_RADIUS) || (auth_rule[i] ==  USERAUTH_AUTH_TACACS))
            {
                if (CGI_AUTH_CheckTempRadiusBase(username, password, &temp_privilege))
                {
                    privilege = temp_privilege;
                    return privilege;
                }
            }
            else
            {
                privilege = -1;
            }
        } /* end for (i = 0; i< USERAUTH_NUMBER_Of_AUTH_METHOD;i++) */
    }

    /* check user_id & password */
    if (USERAUTH_PMGR_LoginAuthBySessionType(username,
                                             password,
                                             sess_type,
                                             0,
                                             rem_ip_addr,
                                             auth_result_p))
    {
#if (SYS_CPNT_WEB_MULTI_PRIVILEGE_LEVEL == TRUE)
        privilege = auth_result_p->privilege;
#else
        if (auth_result_p->privilege == USERAUTH_ADMIN_USER_PRIVILEGE)
        {
            privilege = USERAUTH_ADMIN_USER_PRIVILEGE;
        }
        else
        {
            privilege = USERAUTH_GUEST_USER_PRIVILEGE;
        }
#endif /* #if (SYS_CPNT_WEB_MULTI_PRIVILEGE_LEVEL == TRUE) */
        CGI_AUTH_SetTempRadiusBaseEntry(username, password, privilege);
    }
    else
    {
        privilege = -1;
    }

#else /* if (SYS_CPNT_TACACS == TRUE) */
    if (!USERAUTH_PMGR_GetAuthMethod(&auth_rule))
    {
        return (-1);
    }

    switch(auth_rule)
    {
        case USERAUTH_AUTH_LOCAL_ONLY:
            strcpy(User.username, username);
            is_username_exist = USERAUTH_PMGR_GetLoginLocalUser(&User);

            if ((is_username_exist) &&
                (memcmp(encode_password, User.password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN) == 0))
            {
                privilege = User.privilege;
            }
            else
            {
                privilege = -1;
            }
            break;

        case USERAUTH_AUTH_REMOTE_ONLY:

            if (CGI_AUTH_CheckTempRadiusBase(username, password, &temp_privilege))
            {
                privilege = temp_privilege;
                break;
            }

            if (RADIUS_PMGR_Auth_Check(username, password, &radius_privilege) == 0)
            {
                /* success
                 */
                if (radius_privilege == AUTH_ADMINISTRATIVE)
                {
                    privilege = USERAUTH_ADMIN_USER_PRIVILEGE;
                }
                else
                {
                    privilege = USERAUTH_GUEST_USER_PRIVILEGE;
                }

                CGI_AUTH_SetTempRadiusBaseEntry(username, password, privilege);
            }
            else
            {
                privilege = -1;
            }
            break;

        case USERAUTH_AUTH_REMOTE_THEN_LOCAL:
            if (CGI_AUTH_CheckTempRadiusBase(username, password, &temp_privilege))
            {
                privilege = temp_privilege;
                break;
            }

            if (RADIUS_PMGR_Auth_Check(username, password, &radius_privilege) == 0)
            {
                /* success
                 */
                if (radius_privilege == AUTH_ADMINISTRATIVE)
                {
                    privilege = USERAUTH_ADMIN_USER_PRIVILEGE;
                }
                else
                {
                    privilege = USERAUTH_GUEST_USER_PRIVILEGE;
                }

                CGI_AUTH_SetTempRadiusBaseEntry(username, password, privilege);
            }
            else
            {
                strcpy(User.username, username);
                is_username_exist = USERAUTH_PMGR_GetLoginLocalUser(&User);

                if ((is_username_exist) &&
                    (memcmp (encode_password, User.password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN) == 0))
                {
                    privilege = User.privilege;
                    CGI_AUTH_SetTempRadiusBaseEntry(username, password, privilege);
                }
                else
                {
                    privilege = -1;
                }
            }

            break;

        case USERAUTH_AUTH_LOCAL_THEN_REMOTE:
            strcpy(User.username, username);
            is_username_exist = USERAUTH_PMGR_GetLoginLocalUser(&User);

            if ((is_username_exist) &&
                (memcmp (encode_password, User.password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN) == 0))
            {
                privilege = User.privilege;
                CGI_AUTH_SetTempRadiusBaseEntry(username, password, privilege);
            }
            else
            {
                privilege = -1;
            }

            if( privilege == -1 )
            {
                if(CGI_AUTH_CheckTempRadiusBase(username, password, &temp_privilege))
                {
                    privilege = temp_privilege;
                    break;
                }

                if (RADIUS_PMGR_Auth_Check(username, password, &radius_privilege) == 0)
                {
                    /* success
                     */
                    if (radius_privilege == AUTH_ADMINISTRATIVE)
                    {
                        privilege = USERAUTH_ADMIN_USER_PRIVILEGE;
                    }
                    else
                    {
                        privilege = USERAUTH_GUEST_USER_PRIVILEGE;
                    }

                    CGI_AUTH_SetTempRadiusBaseEntry(username, password, privilege);
                }
                else
                {
                    privilege = -1;
                }
            }

            break;
    }

#endif /* if (SYS_CPNT_TACACS == TRUE)*/
    return (privilege);

}

void CGI_AUTH_InitTempRadiusUserBase()
{
    UI32_T  i;

    for (i=0; i<SYS_ADPT_HTTP_RADIUS_CACHE_SIZE; i++){
        /* Init all string characters to 0 */
        memset(cgi_auth_login_local[i].username, 0, SYS_ADPT_MAX_USER_NAME_LEN);
        memset(cgi_auth_login_local[i].password, 0, SYS_ADPT_MAX_PASSWORD_LEN);
        cgi_auth_login_local[i].privilege = USERAUTH_GUEST_USER_PRIVILEGE;
        cgi_auth_login_local[i].time = 0;
    }
}


/* ------------------------------------------------------------------------
 * FUNCTION NAME - CGI_AUTH_InitSerialNumber
 * ------------------------------------------------------------------------
 * PURPOSE : initial the table - cgi_auth_login_local_serial_number[]
 *
 * INPUT   : None
 *
 *
 * OUTPUT  : None
 * RETURN  : None
 * ------------------------------------------------------------------------
 */
void CGI_AUTH_InitSerialNumber()
{
    /* currently, only implement on Nortel */
    return;
}


BOOL_T CGI_AUTH_CheckTempRadiusBase(/*UI8_T*/const char *username, /*UI8_T*/const char *encode_password, UI32_T *temp_privilege)
{
   UI32_T  i, seconds;

  for (i=0; i<SYS_ADPT_HTTP_RADIUS_CACHE_SIZE; i++){
     if (strcmp(username, (char *) cgi_auth_login_local[i].username) != 0){
      continue;
      }
      if (strcmp(encode_password, (char *) cgi_auth_login_local[i].password) != 0)
      {
#if (SYS_CPNT_TACACS == TRUE)
            continue;
#else
            break;
#endif
      }
    SYS_TIME_GetRealTimeBySec(&seconds);
    if ( (seconds - cgi_auth_login_local[i].time) > SYS_ADPT_HTTP_RADIUS_TIMEOUT){
      break;
    }

    *temp_privilege = cgi_auth_login_local[i].privilege;
     return TRUE;
  }

   return FALSE;
}

void  CGI_AUTH_SetTempRadiusBaseEntry(/*UI8_T*/const char *username, /*UI8_T*/const char *encode_password, UI32_T privilege)
{
   UI32_T  i, seconds, y=0, minTimeEntry = 0xFFFFFFFF;

  SYS_TIME_GetRealTimeBySec(&seconds);

  for (i=0; i<SYS_ADPT_HTTP_RADIUS_CACHE_SIZE; i++){
     if (strcmp(username, (char *)  cgi_auth_login_local[i].username) == 0){
      strcpy((char *) cgi_auth_login_local[i].password, encode_password);
      cgi_auth_login_local[i].privilege = privilege;
      cgi_auth_login_local[i].time = seconds;
      return;
      }else{
      if ( cgi_auth_login_local[i].time < minTimeEntry){
          minTimeEntry = cgi_auth_login_local[i].time;
        y = i;
        }
      }
  }
  /* set values in oldest entry */
  strcpy((char *) cgi_auth_login_local[y].username, username);
  strcpy((char *) cgi_auth_login_local[y].password, encode_password);
  cgi_auth_login_local[y].privilege = privilege;
  cgi_auth_login_local[y].time = seconds;
  return;
}
