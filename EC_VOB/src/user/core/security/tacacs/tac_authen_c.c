/*
     client with full authentication scene
*/
#include "tacacs_type.h"
#include "libtacacs.h"
#include "tacacs_om.h"
#include <stdio.h>

/* NAMING CONSTANT DECLARATIONS
 */
/* define naming constant TAC_AUTHEN_C_DEBUG_MODE
 * to build tac_packet.c with DEBUG version
 * And let following macros print debug messages
 */
 #define TAC_AUTHEN_C_DEBUG_MODE

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef TAC_AUTHEN_C_DEBUG_MODE
#include "backdoor_mgr.h"

    #define TAC_AUTHEN_C_TRACE(fmt, ...)                (TACACS_LOG(TACACS_MGR_GetDebugFlag(TACACS_AUTHEN_C), fmt, ##__VA_ARGS__))

#else

    #define TAC_AUTHEN_C_TRACE(fmt, ...)                ((void)0)

#endif /* TAC_AUTHEN_C_DEBUG_MODE */

I32_T tacacs_main_ascii_login(
    UI32_T server_ip,
    UI8_T *secret,
    UI32_T server_port,
    UI32_T retransmit,
    UI32_T timeout,
    char *user,
    char *passwd,
    int auth_type,
    int auth_priv_lvl,
    TACACS_SessType_T sess_type,
    UI32_T sess_id,
    const L_INET_AddrIp_T *caller_addr
    )
{
    struct session      session_tmp;
    int                 connect_status;
    int                 reply_status;
    int                 retries             = 0;
    char                serv_msg[TACACS_LIB_MAX_LEN_OF_SERVER_MSG + 1];
    char                data_msg[TACACS_LIB_MAX_LEN_OF_DATA + 1];
    char                nas_port[TACACS_TYPE_MAX_LEN_OF_NAS_PORT + 1];
    char                rem_addr[TACACS_TYPE_MAX_LEN_OF_REM_ADDR + 1];

    TAC_AUTHEN_C_TRACE("server_ip=%lu, secret=%s, server_port=%lu, retransmit=%lu, timeout=%lu, "
                       "user=%s, password=%s, auth_type=%d, auth_priv_lvl=%d, sess_type=%d, sess_id=%lu",
                       server_ip, secret, server_port, retransmit, timeout,
                       user, passwd, auth_type, auth_priv_lvl, sess_type, sess_id);

    if (    (FALSE == TAC_UTILS_GetNasPort(sess_type, sess_id, nas_port, sizeof(nas_port)))
        ||  (FALSE == TAC_UTILS_GetRemAddr(sess_type, caller_addr, rem_addr, sizeof(rem_addr)))
       )
    {
        TAC_AUTHEN_C_TRACE("Invalid session type");
        return TAC_PLUS_AUTHEN_STATUS_FAIL;
    }

    TAC_AUTHEN_C_TRACE("nas_port=%s, rem_addr=%s", nas_port, rem_addr);

    while(1)
    {
        /* initiate connection */
        connect_status = tac_connect_nonblocking(server_ip, secret, server_port, timeout, &session_tmp);
        if (connect_status == -1) /*tac_connect_nonb timeout*/
        {
            if (retries++ >= retransmit)
            {
                TAC_AUTHEN_C_TRACE("TACACS/AUTHEN: Connection error");
                return TAC_PLUS_AUTHEN_CONNECT_FAIL;
            }
            continue;
        }
        else if (connect_status == 0) /*tac_connect_nonb failure*/
        {
            TAC_AUTHEN_C_TRACE("TACACS/AUTHEN: Connection error");
            return TAC_PLUS_AUTHEN_CONNECT_FAIL;
        }

        /* send authentication START packet */
        if (1 != tac_authen_send_start(&session_tmp, nas_port, rem_addr, "", auth_type, "", auth_priv_lvl, secret))
        {
            TAC_AUTHEN_C_TRACE("TACACS/ATHEN: Fail to send START packet port=%s\r\n", PORT);
            tac_close(&session_tmp);
            if (retries++ >= retransmit)
            {
                return TAC_PLUS_AUTHEN_CONNECT_FAIL;
            }
            continue;
        }

        /* recv authentication REPLY packet */
        reply_status = tac_authen_get_reply(&session_tmp, serv_msg, data_msg, secret);
        if (TAC_PLUS_AUTHEN_STATUS_GETUSER != reply_status)
        {
            TAC_AUTHEN_C_TRACE("TACACS/ATHEN: Expect reply GETUSER, not (%s)\r\n", tac_print_authen_status(reply_status));
            tac_close(&session_tmp);
            return TAC_PLUS_AUTHEN_STATUS_FAIL;
        }

        /* send authentication CONTINUE packet */
        if (1 != tac_authen_send_cont(&session_tmp,user,"",secret))
        {
            TAC_AUTHEN_C_TRACE("TACACS/ATHEN: Fail to send CONTINUE packet with username=%s\r\n", user);
            tac_close(&session_tmp);
            if (retries++ >= retransmit)
            {
                return TAC_PLUS_AUTHEN_CONNECT_FAIL;
            }
            continue;
        }

        /* recv authentication REPLY packet */
        reply_status = tac_authen_get_reply(&session_tmp,serv_msg,data_msg,secret);
        if (TAC_PLUS_AUTHEN_STATUS_GETPASS != reply_status)
        {
            TAC_AUTHEN_C_TRACE("TACACS/ATHEN: expect reply GETPASS, not (%s)\r\n", tac_print_authen_status(reply_status));
            tac_close(&session_tmp);
            return TAC_PLUS_AUTHEN_STATUS_FAIL;
        }

        /* send authentication CONTINUE packet */
        if (1 != tac_authen_send_cont(&session_tmp,passwd,"",secret))
        {
            TAC_AUTHEN_C_TRACE("TACACS/ATHEN: Fail to send CONTINUE packet with password string=%s\r\n",passwd);
            tac_close(&session_tmp);
            if (retries++ >= retransmit)
            {
                return TAC_PLUS_AUTHEN_CONNECT_FAIL;
            }
            continue;
        }

        /* recv authentication REPLY packet */
        reply_status = tac_authen_get_reply(&session_tmp,serv_msg,data_msg,secret);

        /* now we are disconnect */
        tac_close(&session_tmp);
        break;
    }/* while(1) */

    if(reply_status == TAC_PLUS_AUTHEN_STATUS_PASS)
        return TAC_PLUS_AUTHEN_STATUS_PASS;

    return TAC_PLUS_AUTHEN_STATUS_FAIL;
}

I32_T tacacs_main_enable_requests(
    UI32_T server_ip,
    UI8_T *secret,
    UI32_T server_port,
    UI32_T retransmit,
    UI32_T timeout,
    char *username,
    char *passwd,
    int auth_type,
    int auth_priv_lvl,
    TACACS_SessType_T sess_type,
    UI32_T sess_id,
    const L_INET_AddrIp_T *caller_addr)
{
    struct session      session_tmp;
    int                 connect_status;
    int                 reply_status;
    int                 retries             = 0;
    char                serv_msg[TACACS_LIB_MAX_LEN_OF_SERVER_MSG + 1];
    char                data_msg[TACACS_LIB_MAX_LEN_OF_DATA + 1];
    char                nas_port[TACACS_TYPE_MAX_LEN_OF_NAS_PORT + 1];
    char                rem_addr[TACACS_TYPE_MAX_LEN_OF_REM_ADDR + 1];

    TAC_AUTHEN_C_TRACE("server_ip=%lu, secret=%s, server_port==%lu, retransmit=%lu, timeout=%lu, "
                       "user=%s, password=%s, auth_type=%d, auth_priv_lvl=%d, sess_type=%d, sess_id=%lu",
                       server_ip, secret, server_port, retransmit, timeout,
                       username, passwd, auth_type, auth_priv_lvl, sess_type, sess_id);

    if (    (FALSE == TAC_UTILS_GetNasPort(sess_type, sess_id, nas_port, sizeof(nas_port)))
        ||  (FALSE == TAC_UTILS_GetRemAddr(sess_type, caller_addr, rem_addr, sizeof(rem_addr)))
       )
    {
        TAC_AUTHEN_C_TRACE("Invalid session type");
        return TAC_PLUS_AUTHEN_STATUS_FAIL;
    }

    TAC_AUTHEN_C_TRACE("nas_port=%s, rem_addr=%s", nas_port, rem_addr);

    while(1)
    {
        /* initiate connection */
        connect_status = tac_connect_nonblocking(server_ip, secret, server_port, timeout, &session_tmp);
        if (connect_status == -1) /*tac_connect_nonb timeout*/
        {
            if (retries++ >= retransmit)
            {
                TAC_AUTHEN_C_TRACE("TACACS/AUTHEN: Connection error");
                tac_close(&session_tmp);
                return TAC_PLUS_AUTHEN_CONNECT_FAIL;
            }
            continue;
        }
        else if (connect_status == 0) /*tac_connect_nonb failure*/
        {
            TAC_AUTHEN_C_TRACE("TACACS/AUTHEN: Connection error");
            tac_close(&session_tmp);
            return TAC_PLUS_AUTHEN_CONNECT_FAIL;
        }

        TAC_AUTHEN_C_TRACE("TACACS/ATHEN: auth_priv_lvl=%ld", auth_priv_lvl);

        /* send authentication START packet */
        if (1 != tac_authen_send_start(&session_tmp, nas_port, rem_addr, username/*""*/,auth_type,"",auth_priv_lvl,secret))
        {
            TAC_AUTHEN_C_TRACE("TACACS/ATHEN: Fail to send START packet port=%s\r\n", PORT);
            tac_close(&session_tmp);
            if (retries++ >= retransmit)
            {
                return TAC_PLUS_AUTHEN_CONNECT_FAIL;
            }
            continue;
        }

        /* recv authentication REPLY packet */
        reply_status = tac_authen_get_reply(&session_tmp,serv_msg,data_msg,secret);
        if (TAC_PLUS_AUTHEN_STATUS_GETPASS != reply_status)
        {
            TAC_AUTHEN_C_TRACE("TACACS/ATHEN: Expect reply GETPASS, not (%s)", tac_print_authen_status(reply_status));
            tac_close(&session_tmp);
            return TAC_PLUS_AUTHEN_STATUS_FAIL;
        }

        /* send authentication CONTINUE packet */
        if (1 != tac_authen_send_cont(&session_tmp,passwd,"",secret))
        {
            TAC_AUTHEN_C_TRACE("TACACS/ATHEN: Fail to send CONTINUE packet with password string=%s\r\n",passwd);
            tac_close(&session_tmp);
            if (retries++ >= retransmit)
            {
                return TAC_PLUS_AUTHEN_CONNECT_FAIL;
            }
            continue;
        }

        /* recv authentication REPLY packet */
        reply_status=tac_authen_get_reply(&session_tmp,serv_msg,data_msg,secret);

        /* now we are disconnect */
        tac_close(&session_tmp);

        break;
    }

    if (reply_status == TAC_PLUS_AUTHEN_STATUS_PASS)
        return TAC_PLUS_AUTHEN_STATUS_PASS;

    return TAC_PLUS_AUTHEN_STATUS_FAIL;
}
