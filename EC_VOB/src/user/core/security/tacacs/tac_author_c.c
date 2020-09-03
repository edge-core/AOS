#include <string.h>
#include <stdlib.h>
#include "libtacacs.h"
#include "tacacs_mgr.h"
#include "tac_author_c.h"

#if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE) || (SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD == SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_AUTHORIZATION)

/* NAMING CONSTANT DECLARATIONS
 */
/* define naming constant TAC_AUTHOR_C_DEBUG_MODE
 * to build tac_packet.c with DEBUG version
 * And let following macros print debug messages
 */
 #define TAC_AUTHOR_C_DEBUG_MODE

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef TAC_AUTHOR_C_DEBUG_MODE

#include "backdoor_mgr.h"

    #define TAC_AUTHOR_C_TRACE(fmt, ...)                  (TACACS_LOG(TACACS_MGR_GetDebugFlag(TACACS_AUTHOR_C), fmt, ##__VA_ARGS__))

#else
    #define TAC_AUTHOR_C_TRACE(fmt, ...)                  ((void)0)

#endif /* TAC_AUTHOR_C_DEBUG_MODE */

/* define Attribute-Value pair string */
#define TACACS_LIB_AV_PAIR_SERVICE_SHELL                    "service=shell"
#define TACACS_LIB_AV_PAIR_SHELL_CMD_OPTIONAL               "cmd*"
#define TACACS_LIB_AV_PAIR_CMD_ARG_CR                       "<cr>"


/* define attribute strings used in av pairs */
#define TACACS_LIB_ATTRIBUTE_PRIV_LVL                       "priv-lvl"
#define TACACS_LIB_ATTRIBUTE_SERVICE                        "service"
#define TACACS_LIB_ATTRIBUTE_CMD                            "cmd"

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - tacacs_author_shell_service
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will do shell service authorization.
 * INPUT    : request
 * OUTPUT   : reply
 * RETURN   : TRUE -- success /FALSE -- failure
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
BOOL_T tacacs_author_shell_service(TACACS_AuthorRequest_T *request, TACACS_AuthorReply_T *reply)
{
    struct session  session_buf;
    char *req_av_pair_ar[TACACS_LIB_MAX_NBR_OF_AV_PAIRS];
    const char      *check_list_ar[] = {
                            TACACS_LIB_ATTRIBUTE_PRIV_LVL,
                            TACACS_LIB_ATTRIBUTE_SERVICE,
                            TACACS_LIB_ATTRIBUTE_CMD,
                            NULL /* indicate no more string */
                        };

    /* the last array element MUST be NULL implies no more string */
    char            *reply_av_pair_ar[TACACS_LIB_MAX_NBR_OF_AV_PAIRS];
    char            *str_p;
    I32_T           av_index; /* could be negtive */
    int             status;
    int             connect_status;
    int             retries = 0;
    int             method;
    char            port[TACACS_TYPE_MAX_LEN_OF_NAS_PORT + 1];
    char            rem_addr[TACACS_TYPE_MAX_LEN_OF_REM_ADDR + 1];
    char            msg[TACACS_LIB_MAX_LEN_OF_SERVER_MSG + 1];
    char            data[TACACS_LIB_MAX_LEN_OF_DATA + 1];
#if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION_COMMAND == TRUE)
    char            cmd_str[SYS_ADPT_CLI_MAX_BUFSIZE + 4 +1]   = {0}; /*MAX command length + "cmd=" + '\0' */
    char            arg_str[SYS_ADPT_CLI_MAX_BUFSIZE + 4 +1]   = {0};
#endif /* #if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION_COMMAND == TRUE) */


    TAC_AUTHOR_C_TRACE("ifindex=%lu, sess_type=%d, user=%s, server_ip=%lu, server_port=%lu, "
                       "retransmit=%lu, timeout=%lu, secret=%s, curr_privilege=%lu, authen_by_whom=%d",
                       request->ifindex, request->sess_type, request->user_name, request->server_ip, request->server_port,
                       request->retransmit, request->timeout, request->secret, request->current_privilege, request->authen_by_whom);

    method = TACACS_LIB_ConvertAuthMethod(request->authen_by_whom);

    TAC_UTILS_GetNasPort(request->sess_type,
                         ((request->sess_type == TACACS_SESS_TYPE_CONSOLE) ? 0 : request->ifindex-1),
                         port,
                         sizeof(port));

    TAC_UTILS_GetRemAddr(request->sess_type,
                         &request->rem_ip_addr,
                         rem_addr,
                         sizeof(rem_addr));

    TAC_AUTHOR_C_TRACE("method=%d, port=%s, rem_addr=%s", method, port, rem_addr);

    memset(reply_av_pair_ar, 0, sizeof(reply_av_pair_ar));
    memset(req_av_pair_ar, 0, sizeof(req_av_pair_ar));

    req_av_pair_ar[0] = TACACS_LIB_AV_PAIR_SERVICE_SHELL;

    TAC_AUTHOR_C_TRACE("arg[0]: %s", req_av_pair_ar[0]);

#if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION_COMMAND == TRUE)
    if ('\0' == request->command[0])
    {
        req_av_pair_ar[1] = TACACS_LIB_AV_PAIR_SHELL_CMD_OPTIONAL;

        TAC_AUTHOR_C_TRACE("arg[1]: %s", req_av_pair_ar[1]);
    }
    else
    {
        size_t arg_pos = strcspn(request->command, " \t");
        size_t size = arg_pos+4+1 > sizeof(cmd_str) ? sizeof(cmd_str) : arg_pos+4+1;

        snprintf(cmd_str, size, "cmd=%s", request->command);
        cmd_str[ size ] = '\0';

        req_av_pair_ar[1] = cmd_str;

        TAC_AUTHOR_C_TRACE("arg[1]: %s", req_av_pair_ar[1]);

        if (arg_pos < strlen(request->command))
        {
            snprintf(arg_str, sizeof(arg_str), "cmd-arg=%s", request->command + arg_pos + 1);
            cmd_str[ sizeof(arg_str)-1 ] = '\0';

            req_av_pair_ar[2] = arg_str;

            TAC_AUTHOR_C_TRACE("arg[2]: %s", req_av_pair_ar[2]);

            req_av_pair_ar[3] = TACACS_LIB_AV_PAIR_CMD_ARG_CR;

            TAC_AUTHOR_C_TRACE("arg[3]: %s", req_av_pair_ar[3]);
        }
    }
#else
    req_av_pair_ar[1] = TACACS_LIB_AV_PAIR_SHELL_CMD_OPTIONAL;

    TAC_AUTHOR_C_TRACE("arg[1]: %s", req_av_pair_ar[1]);
#endif /* #if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION_COMMAND == TRUE) */

    while(1)
    {
        /* initiate connection */
        connect_status = tac_connect_nonblocking(request->server_ip, request->secret, request->server_port, request->timeout, &session_buf);
        if (connect_status == -1)       /*tac_connect_nonb timeout*/
        {
            if (retries++ >= request->retransmit)
            {
                TAC_AUTHOR_C_TRACE("TACACS/AUTHOR: Connection error");
                reply->return_type = TACACS_AuthorRequest_TIMEOUT; /*shift to next server*/
                return FALSE;
            }
            continue;
        }
        else if (connect_status == 0) /*tac_connect_nonb failure*/
        {
            TAC_AUTHOR_C_TRACE("TACACS/AUTHOR: Connection error");
            reply->return_type = TACACS_AuthorRequest_TIMEOUT; /*shift to next server*/
            return FALSE;
        }

        /* send Authorization request */
        if (1 != tac_author_send_request(&session_buf,
                                         method,
                                         TACACS_PRIVILEGE_OF_ADMIN,
                                         TAC_PLUS_AUTHEN_TYPE_ASCII,
                                         TAC_PLUS_AUTHEN_SVC_LOGIN,
                                         request->user_name,
                                         port,
                                         rem_addr,
                                         (char **)req_av_pair_ar,
                                         request->secret))
        {
            TAC_AUTHOR_C_TRACE("TACACS/AUTHOR: Fail to send request");
            tac_close(&session_buf);
            if (retries++ >= request->retransmit)
            {
                reply->return_type = TACACS_AuthorRequest_TIMEOUT;
                return FALSE;
            }
            continue;
        }

        memset(req_av_pair_ar, 0, sizeof(req_av_pair_ar));

        /* MUST remember free returned AV pairs memory */
        status = tac_author_get_response(&session_buf, msg, data,
                    reply_av_pair_ar, request->secret);
        tac_close(&session_buf);
        break;
    } /*end of while(1)*/

    /* check returned status */
    if ((TAC_PLUS_AUTHOR_STATUS_PASS_ADD != status) &&
        (TAC_PLUS_AUTHOR_STATUS_PASS_REPL != status))
    {
        TAC_AUTHOR_C_TRACE("authorization failed(%d)", status);
        reply->return_type = TACACS_AuthorRequest_FAILED;
        TACACS_LIB_FreeAvPairs(reply_av_pair_ar);
        return FALSE;
    }

    /* check returned av pairs */
    if (FALSE == TACACS_LIB_CheckReturnedAvPairs(check_list_ar, reply_av_pair_ar))
    {
        TAC_AUTHOR_C_TRACE("bad returned av pairs");
        reply->return_type = TACACS_AuthorRequest_FAILED;
        TACACS_LIB_FreeAvPairs(reply_av_pair_ar);
        return FALSE;
    }

    av_index = TACACS_LIB_FindAttributeInAvPairs(
                TACACS_LIB_ATTRIBUTE_PRIV_LVL, reply_av_pair_ar);

    /* if server does not return priv_lvl */
    if (0 > av_index)
    {
        TAC_AUTHOR_C_TRACE("does not return priv_lvl");
        reply->return_type = TACACS_AuthorRequest_SUCCEEDED_WITH_NO_PRIV;
    }
    else
    {
        str_p = TACACS_LIB_FindSeparatedCharInAvPair(reply_av_pair_ar[av_index]);
        if (NULL == str_p)
        {
            /* should not go here because this function call
             * TACACS_LIB_CheckReturnedAvPairs() to check returned
             * av pairs already
             */
            TAC_AUTHOR_C_TRACE("should not go here");
            reply->return_type = TACACS_AuthorRequest_FAILED;
            TACACS_LIB_FreeAvPairs(reply_av_pair_ar);
            return FALSE;
        }

        ++str_p; /* move pointer to next character */
        reply->new_privilege = atoi(str_p);
        reply->return_type = TACACS_AuthorRequest_SUCCEEDED;

        TAC_AUTHOR_C_TRACE("new priv_lvl(%s) = %lu",
            str_p, reply->new_privilege);
    }

    TACACS_LIB_FreeAvPairs(reply_av_pair_ar);
    return TRUE;
}


#endif /* SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE || SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD == SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_AUTHORIZATION */
