/* ----------------------------------------------------------------------
 * FILE NAME: http\htReqest.c

 * AUTHOR: Integrated Systems, Inc
 * MODIFIER: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Library for core of HTTP server.
 * File for the "Process Request" task.

 * HISTORY:
 * 1997-10-03 (Fri): Modified by Daniel K. Chung.
 * 1997-10-03 (Fri): Changed task parameters (HTTP_OLDTASK).
 * 1997-12-29 (Tue): "Fileless" is not a task (HTTP_FILELESSTASK).
 * 1998-02-24 (Tue): Checks for duplicate environment (HTTP_DUPENV).
 * 1998-04-30 (Thu): Renamed tasks and semaphores (HTTP_OLDNAME).
 * 1998-04-30 (Thu): Removed "httpcfg" (HTTP_CFG).
 * 1998-04-30 (Thu): Removed POST request (HTTP_POST).
 * 1998-04-30 (Thu): Removed file actions (HTTP_FILE).
 * 1998-05-08 (Fri): Name changed from httpmisc.c to htReqest.c.
 * 1998-05-13 (Wed): Reimplemented POST with limited size (HTTP_POST).
 * 1998-05-28 (Thu): Removed global string arrays (HTTP_STRING).
 * 1998-05-28 (Thu): Removed build-header (HTTP_HEADER).
 * 1998-07-01 (Wed): Changed large stack arrays to malloc (HTTP_STACK).
 * 1998-07-01 (Wed): Actually removed unnecessary code (HTTP_<all>, SC_PHILE).
 * 1998-07-30 (Thu): Removed sprintf (causes problems with PowerPC).
 * 1998-07-31 (Fri): Converted all large arrays on stack to malloc.
 * 1998-08-03 (Mon): Converted structures on stack to malloc.
 * 1998-08-03 (Mon): Updated "http_standard_env", ignore unnecessary env.
 * 1998-08-06 (Thu): Added "free (-1)" before task exit for PPC (HTTP_PPC).
 * 1998-09-17 (Wed): Added CGI semaphore for Tomcat platform (HTTP_TOMCAT).
 * 1998-11-10 (Tue): "Post" data is read up to "content length" only.
 * 1998-11-26 (Thu): Set socket to non-blocking state (HTTP_NONBLOCK).
 * 1998-11-27 (Fri): Removed code which stores unnecessary env.
 * 1998-12-23 (Wed): Don't show some error messages.
 * 1999-04-28 (Wed): Use function call to process request (HTTP_REQTASK).

 * COPYRIGHT (C) Accton Technology Corporation, 1997.
 * ---------------------------------------------------------------------- */

/***********************************************************************/
/*                                                                     */
/*   MODULE: sys\libc\src\http\httpmisc.c                              */
/*   DATE:   96/04/23                                                  */
/*   PURPOSE:                                                          */
/*                                                                     */
/*---------------------------------------------------------------------*/
/*                                                                     */
/*      Copyright 1991 - 1993, Integrated Systems, Inc.                */
/*          ALL RIGHTS RESERVED                                        */
/*                                                                     */
/*   Permission is hereby granted to licensees of Integrated Systems,  */
/*   Inc. products to use or abstract this computer program for the    */
/*   sole purpose of implementing a product based on Integrated        */
/*   Systems, Inc. products.   No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in       */
/*   whole, are granted.                                               */
/*                                                                     */
/*   Integrated Systems, Inc. makes no representation or warranties    */
/*   with respect to the performance of this computer program, and     */
/*   specifically disclaims any responsibility for any damages,        */
/*   special or consequential, connected with the use of this program. */
/*                                                                     */
/***********************************************************************/
#include "http_loc.h"

#include "cgi_main.h"

#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"

#include "ip_lib.h"

#if (SYS_CPNT_CLUSTER == TRUE)
#include "cluster_pom.h"
#define HTTP_CLUSTER_MAX_RETRY_TIME      20
#define HTTP_CLUSTER_MAX_RELAY_BUF_SIZE 2048
#endif /* SYS_CPNT_CLUSTER */

#if(SYS_CPNT_WEBAUTH == TRUE)
#include "webauth_type.h"
#include "webauth_mgr.h"
#include "netcfg_pom_ip.h"
#include "netcfg_pmgr_route.h"
#include "netcfg_type.h"
#include "iml_pmgr.h"
#include "ipal_types.h"
#include "ipal_route.h"
#endif /* SYS_CPNT_WEBAUTH */

#define s_close(s) HTTP_closesocket(s)

/* state constants for read_header() */
#define NORMAL 		0
#define CR 		     13  	/* ASCII value of CR */
#define LF 		     10	    /* ASCII value of LF */
#define CRLF 		       1
#define CRLFCR 		2
#define CRLFCRLF 	3
#define CRCR               4

/*-----------------------------------------------------------------------
        Function Prototypes
------------------------------------------------------------------------*/

static void http_request_handshake(HTTP_Event_T *event);
static void http_request_headers(HTTP_Event_T *event);

static void err_end(int, HTTP_Request_T *);

static int check_for_end_of_header(HTTP_Request_T *, int *, int *);
static int parse_header(HTTP_Request_T *);
static int parse_method(HTTP_Request_T *);
static int parse_uri(HTTP_Request_T *);
static int parse_request(HTTP_Connection_T *http_connection, HTTP_Request_T *);

#if (SYS_CPNT_CLUSTER == TRUE)
static BOOL_T http_relay_action(HTTP_Request_T *, unsigned int, HTTP_Connection_T *http_connection);
static BOOL_T http_relaying(HTTP_Connection_T *http_connection);
#endif /* SYS_CPNT_CLUSTER */

static int http_request_read_headers(HTTP_Connection_T *http_connection);
static void cleanup_uri(char * request_uri, HTTP_Request_T *);
static int send_error_reply(HTTP_Connection_T *http_connection, HTTP_Request_T * req);

static void set_path(HTTP_Request_T * req);

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if (SYS_CPNT_HTTPS == TRUE)

/**----------------------------------------------------------------------
 * Connect SSL to the accepted socket.
 *
 * @param  req      request structure
 * @param  port_num port number
 * @return          return-value-1 - return value (1) to indicate success
 *                  and (0) to indicate failure.
 * ---------------------------------------------------------------------- */
static UI32_T HTTP_Handshake(HTTP_Request_T *, int);

/**----------------------------------------------------------------------
 * Auto-redirect https homepage when received http request.
 *
 * @param  src              http request string.
 * @param  dest
 * @param  secure_port
 * @param  is_port_changed
 * @return                  return-value-1 - Auto-redirect command string.
 * ---------------------------------------------------------------------- */
static BOOL_T HTTP_Get_Host_Ip_From_Request(char *, char **, int, BOOL_T);

#endif /* SYS_CPNT_HTTPS */

/**----------------------------------------------------------------------
 * get port number from sockaddr
 *
 * @return  -1 if failed
 * ---------------------------------------------------------------------- */
static int HTTP_GetSockPort(struct sockaddr *sa);

static BOOL_T get_addresses(HTTP_Connection_T *conn_info_p, UI32_T *sock_address_p, UI32_T *sock_port_p, L_INET_AddrIp_T *sock_inet_addr_p, UI32_T *peer_address_p, UI32_T *peer_port_p, L_INET_AddrIp_T *peer_sock_p);

#if (SYS_CPNT_WEBAUTH == TRUE)
static BOOL_T get_dut_address(UI32_T peer_addr, UI32_T *dut_addr_p);
static BOOL_T http_process_webauth(HTTP_Connection_T *http_connection, UI32_T peer_addr, UI32_T lport, HTTP_Request_T *req_p);
static BOOL_T set_peer_addr_to_env(HTTP_Request_T *req_p, HTTP_Connection_T *conn_info_p, UI32_T address);
static BOOL_T set_lport_to_env(HTTP_Request_T *req_p, HTTP_Connection_T *conn_info_p, UI32_T port);
static BOOL_T is_webauth_url(HTTP_Request_T *req_p);
#endif /* SYS_CPNT_WEBAUTH */

/*---------------------------------------------------------------------------------
        Global Variables
----------------------------------------------------------------------------------*/
/*isiah. 2002-10-21, add static  */
static const char *http_standard_env [] = {
        "CONTENT_LENGTH",
        "CONTENT_TYPE",
        "PATH_INFO",
        "METHOD",
        "AUTHORIZATION",
        "HOST",
        "COOKIE", /* ivan_liao 2006/4/10 */
        (char *) 0
};

static BOOL_T get_addresses(HTTP_Connection_T *conn_info_p, UI32_T *sock_address_p, UI32_T *sock_port_p, L_INET_AddrIp_T *sock_inet_address_p, UI32_T *peer_address_p, UI32_T *peer_port_p, L_INET_AddrIp_T *peer_sock_address_p)
{
    int                 sockfd;
    int                 ret;
    struct sockaddr_in6 sock_addr;
    socklen_t           sock_addr_size;
    struct sockaddr_in6 peer_sock_addr;
    socklen_t           peer_sock_addr_size;

    sockfd = conn_info_p->fds[HTTP_CONN_FD_NET];

    sock_addr_size = sizeof(sock_addr);
    ret = getsockname(sockfd, (struct sockaddr *)&sock_addr, &sock_addr_size);
    if (-1 == ret)
    {
        return FALSE;
    }

    L_INET_SockaddrToInaddr((struct sockaddr *)&sock_addr, sock_inet_address_p);
    IP_LIB_ArraytoUI32(sock_inet_address_p->addr, sock_address_p);
    *sock_port_p = HTTP_GetSockPort((struct sockaddr *)&sock_addr);

    peer_sock_addr_size = sizeof(peer_sock_addr);
    ret = getpeername(sockfd, (struct sockaddr *)&peer_sock_addr, &peer_sock_addr_size);
    if (-1 == ret)
    {
        return FALSE;
    }

    L_INET_SockaddrToInaddr((struct sockaddr *)&peer_sock_addr, peer_sock_address_p);
    IP_LIB_ArraytoUI32(peer_sock_address_p->addr, peer_address_p);
    *peer_port_p = HTTP_GetSockPort((struct sockaddr *)&peer_sock_addr);

    return TRUE;
}

#if (SYS_CPNT_WEBAUTH == TRUE)
#define WEBAUTH_REDIRECT    1
#define WEBAUTH_TABLE_FULL  2
#define WEBAUTH_BLACK_HOST  3

static BOOL_T get_dut_address(UI32_T peer_addr, UI32_T *dut_addr_p)
{
    L_INET_AddrIp_T src_ip;
    L_INET_AddrIp_T dst_ip;
    L_INET_AddrIp_T nexthop_ip;
    UI32_T out_ifindex;

    memset(&dst_ip, 0x0, sizeof(dst_ip));
    memset(&src_ip, 0x0, sizeof(src_ip));

    dst_ip.type = L_INET_ADDR_TYPE_IPV4;
    dst_ip.addrlen = SYS_ADPT_IPV4_ADDR_LEN;

    IP_LIB_UI32toArray(peer_addr, dst_ip.addr);

    if (IPAL_RESULT_OK != IPAL_ROUTE_RouteLookup(&dst_ip, &src_ip, &nexthop_ip, &out_ifindex))
    {
        return FALSE;
    }

    IP_LIB_ArraytoUI32(src_ip.addr, dut_addr_p);
    return TRUE;
}

/**----------------------------------------------------------------------
 * This function will process webauth redirect job
 *
 * @param  peer_addr    peer address
 * @param  l_port       incoming port
 * @param  req_p        request pointer
 * @return              TRUE/FALSE
 * ---------------------------------------------------------------------- */
static BOOL_T http_process_webauth(HTTP_Connection_T *http_connection, UI32_T peer_addr, UI32_T lport, HTTP_Request_T *req_p)
{
    UI16_T  success_count;
    BOOL_T  send_reply = 1;
    char    tmpbuf_ar[2]={0};

    if (WEBAUTH_MGR_CheckIPIsBlackByLPort(peer_addr, lport) == WEBAUTH_TYPE_RETURN_OK)
    {
        sprintf(tmpbuf_ar, "%d", WEBAUTH_BLACK_HOST);
        set_env(req_p->envcfg, "WEBAUTH_TYPE", tmpbuf_ar);
    }
    else
    {
        if (WEBAUTH_MGR_GetSuccessCountByLPort(lport, &success_count) == WEBAUTH_TYPE_RETURN_OK)
        {
            if (success_count >= SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT)
            {
                sprintf(tmpbuf_ar, "%d", WEBAUTH_TABLE_FULL);
                set_env(req_p->envcfg, "WEBAUTH_TYPE", tmpbuf_ar);
            }
            else
            {
                /* for redirect, must keep path_info, so pass this check
                 * for later
                 */
                sprintf(tmpbuf_ar, "%d", WEBAUTH_REDIRECT);
                set_env(req_p->envcfg, "WEBAUTH_TYPE", tmpbuf_ar);
            }
        }
        else /* can't get success count */
        {
            send_reply = 0;
        }
    }
    if (send_reply == 1)
    {
        req_p->status = R_MOVED_TEMP;
        send_error_reply(http_connection, req_p);
    }
    return TRUE;
}

static BOOL_T set_peer_addr_to_env(HTTP_Request_T *req_p, HTTP_Connection_T *conn_info_p, UI32_T address)
{
    char tmp_str_ar[16] = {0};

    L_INET_Ntoa(address, (UI8_T *)tmp_str_ar);
    if (0 != set_env(req_p->envcfg, "IP_ADDR", tmp_str_ar))
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL_T set_lport_to_env(HTTP_Request_T *req_p, HTTP_Connection_T *conn_info_p, UI32_T port)
{
    char tmp_str_ar[16] = {0};

    sprintf(tmp_str_ar, "%lu", port);
    if (0 != set_env(req_p->envcfg, "SRC_PORT", tmp_str_ar))
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL_T is_webauth_url(HTTP_Request_T *req_p)
{
    char *uri_p;

    uri_p = get_env(req_p->envcfg, "URI");

    if (uri_p == NULL)
    {
        return FALSE;
    }

    if (   (strncmp(uri_p, "/webauth/login.htm", 18) != 0)
        && (strncmp(uri_p, "/webauth/login_fail.htm", 23) != 0)
        && (strncmp(uri_p, "/webauth/login_fail_held.htm", 28) != 0)
        && (strncmp(uri_p, "/webauth/login_full.htm", 23) != 0)
        && (strncmp(uri_p, "/webauth/login_success.htm", 26) != 0))
    {
        return FALSE;
    }

    return TRUE;
}
#endif /* SYS_CPNT_WEBAUTH */

void http_process_request(HTTP_Event_T *event)
{
    HTTP_Connection_T *http_connection = (HTTP_Connection_T*) event->data;
    UI32_T                      sock_address;
    UI32_T                      peer_address;
    UI32_T                      sock_port;
    UI32_T                      peer_port;
    L_INET_AddrIp_T             sock_inet_address;
    L_INET_AddrIp_T             peer_inet_address;

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_CONNECTION, 1,
        "Process request, fd(%d)",
        http_connection->fds[HTTP_CONN_FD_NET]);

#if (SYS_CPNT_CLUSTER == TRUE)
    // TODO: This code shall not really need ????
    if( http_connection->is_relaying == TRUE)
    {
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_CONNECTION, 1,
            "http_connection->is_relaying = TRUE, fd(%d)",
            http_connection->fds[HTTP_CONN_FD_NET]);

        if (TRUE == http_relaying(http_connection))
        {
            goto fail;
        }
        else
        {
            goto fail;
        }
    }
#endif /* SYS_CPNT_CLUSTER */

    http_connection_dbg_check(http_connection);

    if (FALSE == get_addresses(http_connection, &sock_address, &sock_port, &sock_inet_address, &peer_address, &peer_port, &peer_inet_address))
    {
        goto fail;
    }

#if (SYS_CPNT_WEBAUTH == TRUE)
    if (FALSE == set_peer_addr_to_env(http_connection->req, http_connection, peer_address))
    {
        goto fail;
    }
#endif /* SYS_CPNT_WEBAUTH */

    event->handler = http_request_handshake;
    http_event_add(http_connection->worker->event_ctx, event, 0);
    return;

fail:
    http_connection->done = 1;
    http_connection->keepalive = 0;

    http_request_finalize(event);

    return;
}

void http_request_finalize(HTTP_Event_T *event)
{
    HTTP_Connection_T *http_connection = (HTTP_Connection_T *)event->data;

    if (http_connection->done &&
        !http_connection->res->is_send_eof)
    {
        send_error_reply(http_connection, http_connection->req);
    }

    http_connection_finalize(event);
}

static void http_request_handshake(HTTP_Event_T *event)
{
    HTTP_Connection_T *http_connection = (HTTP_Connection_T *) event->data;
    http_connection_dbg_check(http_connection);

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_CONNECTION, 1,
        "Handshake, fd(%d)",
        http_connection->fds[HTTP_CONN_FD_NET]);

#if (SYS_CPNT_HTTPS == TRUE)
    {
        UI32_T sock_address;
        UI32_T peer_address;
        UI32_T sock_port;
        UI32_T peer_port;
        L_INET_AddrIp_T sock_inet_address;
        L_INET_AddrIp_T peer_inet_address;

        if (FALSE == get_addresses(http_connection, &sock_address, &sock_port, &sock_inet_address, &peer_address, &peer_port, &peer_inet_address))
        {
            goto fail;
        }

        http_connection->req->Is_Handshake = HTTP_Handshake(http_connection->req, sock_port);
        if (http_connection->req->Is_Handshake == HTTP_HANDSKAHE_FAILURE)
        {
            goto fail;
        }

        if (http_connection->conn_state == HTTPS_CONNECTION)
        {
            L_BIO_socket_assign_ssl(&http_connection->req->bfd, http_connection->req->ssl);
            L_BIO_socket_assign_ssl(&http_connection->res->bfd, http_connection->req->ssl);
        }

        if( http_connection->req->Is_Handshake == HTTP_HANDSKAHE_PORT_FAILURE )
        {
            goto fail;
        }
        if( http_connection->req->Is_Handshake == HTTP_HANDSKAHE_NO_SECOND_HELLO )
        {
            goto fail;
        }
        if( http_connection->req->Is_Handshake == HTTP_HANDSKAHE_NO_PACKET )
        {
            goto fail;
        }
        if( http_connection->req->Is_Handshake == HTTP_HANDSKAHE_BROKEN )
        {
            goto fail;
        }

        if (0)
        {
        fail:
            http_connection->done = 1;
            http_connection->keepalive = 0;

            event->handler = http_request_finalize;
            http_request_finalize(event);
            return;
        }
    }
#endif /* SYS_CPNT_HTTPS */

    ASSERT(http_connection->done != 1);
    ASSERT(http_connection != NULL);

    event->handler = http_request_headers;

    if (http_connection->conn_state == HTTPS_CONNECTION)
    {
        http_event_add(http_connection->worker->event_ctx, event, HTTP_TIMER_INFINITE);
    }
    else
    {
        http_request_headers(event);
    }
}

static void http_request_headers(HTTP_Event_T *event)
{
    UI32_T sock_address;
    UI32_T peer_address;
    UI32_T sock_port;
    UI32_T peer_port;
    L_INET_AddrIp_T sock_inet_address;
    L_INET_AddrIp_T peer_inet_address;
    int    ret;

    HTTP_Connection_T *http_connection = (HTTP_Connection_T *) event->data;
    http_connection_dbg_check(http_connection);

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_CONNECTION, 1,
        "Process headers, fd(%d)",
        http_connection->fds[HTTP_CONN_FD_NET]);

    if (FALSE == get_addresses(http_connection, &sock_address, &sock_port, &sock_inet_address, &peer_address, &peer_port, &peer_inet_address))
    {
        goto fail;
    }

    /* read header data and null terminate it */
    ret = http_request_read_headers(http_connection);
    if (ret == HTTP_AGAIN)
    {
        event->handler = http_request_headers;
        http_event_add(http_connection->worker->event_ctx, event, HTTP_TIMER_INFINITE);
        return;
    }
    else if (ret != HTTP_OK)
    {
        send_error_reply(http_connection, http_connection->req);
        goto fail;
    }

    if (parse_request(http_connection, http_connection->req) == HTTP_ERROR)
    {
        send_error_reply(http_connection, http_connection->req);
        goto fail;
    }

#if (SYS_CPNT_CLUSTER == TRUE)
    {
        int                         cluster_id;
        BOOL_T                      is_relay;
        char                       *temp_str;
        CLUSTER_TYPE_EntityInfo_T   cluster_info;

        is_relay = FALSE;

        /*decide to relay or go through to CGI by checking cluster's ID*/
        temp_str = get_env(http_connection->req->envcfg, "REMOTE_CLUSTER_ID");
        memset(&cluster_info, 0, sizeof(CLUSTER_TYPE_EntityInfo_T));
        CLUSTER_POM_GetClusterInfo(&cluster_info);

        if (temp_str != NULL && cluster_info.role == CLUSTER_TYPE_COMMANDER)
        {
            /* get the cluster_id  */
            cluster_id = atoi(temp_str);
            is_relay = TRUE;
        }

        // FIXME: Security issue !!
        //        A unauth commander can send commander to member directly

        /* do HTTP Relay */
        if (TRUE == is_relay)
        {
            HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_CONNECTION, 1,
                "Cluster, proxying request, fd(%d)",
                http_connection->fds[HTTP_CONN_FD_NET]);

            if (http_relay_action(http_connection->req, cluster_id, http_connection) == FALSE)
            {
                HTTP_LOG_ERROR(HTTP_LOG_LEVEL_NOTICE, HTTP_LOG_MSG_CONNECTION, 1,
                    "Cluster, failed to proxy request, fd(%d)",
                    http_connection->fds[HTTP_CONN_FD_NET]);
                goto fail;
            }

            http_connection->done = 1;
            http_connection->keepalive = 0;

            event->handler = http_request_finalize;
            http_event_add(http_connection->worker->event_ctx, event, 0);
            return;
        }
    }
#endif /* SYS_CPNT_CLUSTER */

#if (SYS_CPNT_WEBAUTH == TRUE)
    {
        UI32_T org_dip;
        UI32_T lport = 1;
        UI8_T status;
        UI8_T port_status;

        /* Return value of IML_PMGR_GetOrgDipAndIngLportBySrcIpAndSrcTcpPort
         *        | src  | actual dst ip  | need to do webauth ?
         *  ------+------+----------------+-------------------------
         *   DUT? | NO   | NO             | TRUE
         *        | NO   | YES            | TRUE
         *        | YES  | NO             | X
         *        | YES  | YES            | FALSE (used for UPnP to get XML)
         */
        if (TRUE == IML_PMGR_GetOrgDipAndIngLportBySrcIpAndSrcTcpPort(peer_address, peer_port, &org_dip, &lport))
        {
            if (   (WEBAUTH_TYPE_RETURN_OK != WEBAUTH_MGR_GetSystemStatus(&status))
                || (WEBAUTH_TYPE_RETURN_OK != WEBAUTH_MGR_GetStatusByLPort(lport, &port_status)))
            {
                goto fail;
            }

            if (   (VAL_webauthEnable_enabled == status)
                && (VAL_webAuthPortConfigStatus_enabled == port_status))
            {
                if (FALSE == set_lport_to_env(http_connection->req, http_connection, lport))
                {
                    /* set "SRC_PORT" env and it will be catnated in query string later.
                     * cgi will use this value to do webauth.
                     */
                    goto fail;
                }

                if (FALSE == is_webauth_url(http_connection->req))
                {
                    /* it is not redirect to webauth page yet (first time)
                     */
                    if (FALSE == WEBAUTH_MGR_IsIPValidByLPort(peer_address, lport))
                    {
                        http_process_webauth(http_connection, peer_address, lport, http_connection->req);
                        goto fail;
                    }
                }

                /* Go here, means the request is redirect by web auth.
                 * It not want to configure the switch, so skip to check
                 * the remote address.
                 */
                goto skip_mgmt_ip_flt;
            }
        }
    }
#endif /* SYS_CPNT_WEBAUTH */

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
    if (MGMT_IP_FLT_IsValidIpFilterAddress(MGMT_IP_FLT_HTTP, &peer_inet_address) == FALSE)
    {
        goto fail;
    }
#endif /* SYS_CPNT_MGMT_IP_FLT */

#if (SYS_CPNT_WEBAUTH == TRUE)
skip_mgmt_ip_flt:
#endif

    event->handler = http_fileless1;
    http_event_add(http_connection->worker->event_ctx, event, 0);
    return;

fail:
    http_connection->done = 1;
    http_connection->keepalive = 0;

    http_request_finalize(event);
}

HTTP_Request_T * http_request_new()
{
    HTTP_Request_T *req;

    req = (HTTP_Request_T *) L_MM_Malloc(sizeof(HTTP_Request_T),
                                         L_MM_USER_ID2(SYS_MODULE_HTTP,
                                                       HTTP_TYPE_TRACE_ID_HTTP_PROCESS_REQUEST));
    if (req == NULL)
    {
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_MEMORY, 1,
                       "%s", "req(Out of memory)");
        goto fail;
    }

    memset(req, 0, sizeof(HTTP_Request_T));

    req->envcfg = (envcfg_t *)L_MM_Malloc(sizeof(envcfg_t),
                                          L_MM_USER_ID2(SYS_MODULE_HTTP,
                                                        HTTP_TYPE_TRACE_ID_HTTP_PROCESS_REQUEST));
    if (req->envcfg == NULL)
    {
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_MEMORY, 1,
                       "%s", "req.envcfg(Out of memory)");
        goto fail;
    }

    init_env(req->envcfg);

    req->query = (envcfg_t *) L_MM_Malloc(sizeof(envcfg_t),
                                          L_MM_USER_ID2(SYS_MODULE_HTTP,
                                                        HTTP_TYPE_TRACE_ID_HTTP_PROCESS_REQUEST));

    if (req->query == NULL)
    {
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_MEMORY, 1,
                       "%s", "req.query(Out of memory)");
        goto fail;
    }

    init_env(req->query);

    return req;

fail:
    http_request_free(&req);

    return NULL;
}

/**----------------------------------------------------------------------
 * Single point of exit from http_process_request.
 * Release a semaphore token used for throttling, and do task cleanup
 * before killing task.
 * ---------------------------------------------------------------------- */
// TODO: rename as http_close_request
void http_request_free(HTTP_Request_T **req_pp)
{
    HTTP_Request_T *req;

    ASSERT(req_pp);
    if (req_pp == NULL)
    {
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_PARAMETER, 1,
                       "req_pp(%s)", "Invalid paameter: null pointer");
        return;
    }

    if (*req_pp == NULL)
    {
        return;
    }

    req = *req_pp;

    if (req->envcfg != NULL)
    {
        free_env(req->envcfg);
        L_MM_Free(req->envcfg);
        req->envcfg = NULL;
    }

    if (req->query != NULL)
    {
        free_env(req->query);
        L_MM_Free(req->query);
        req->query = NULL;
    }

#if (SYS_CPNT_HTTPS == TRUE)
    if (req->ssl != NULL)
    {
        if (req->Is_Handshake == HTTP_HANDSKAHE_SUCCESS)
        {
            SSL_set_shutdown(req->ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
            SSL_shutdown(req->ssl);
        }
        else if (req->Is_Handshake == HTTP_HANDSKAHE_FAILURE)
        {
            SSL_set_shutdown(req->ssl, SSL_RECEIVED_SHUTDOWN);
            SSL_shutdown(req->ssl);
        }

        SSL_free(req->ssl);
        ERR_clear_error();
        ERR_remove_state(0);
        req->ssl = NULL;
    }
#endif /* SYS_CPNT_HTTPS */

    L_MM_Free(req);

    *req_pp = NULL;
}

/**----------------------------------------------------------------------
 * Socket error while sending or receiving header.  Display message
 * and then call httpd_exit to terminate task.
 * Cannot call httpd_exit.  Should return.  Zhong Qiyao, 1998-11-27 (Fri).
 * ---------------------------------------------------------------------- */
static void err_end(int error_code, HTTP_Request_T *req)
{
#ifdef DEBUG_PRINTF
    printf("err_end\n");

    switch(error_code)
    {
        case E_SEND:
            printf("htReqest.c:err_end: E_SEND error\n");
            break;

        case E_RECV:
            printf("htReqest.c:err_end: E_RECV error\n");
            break;
    }
#endif /* DEBUG_PRINTF */
}

/**----------------------------------------------------------------------
 * Read input stream and parse it into a request_struct.
 * Read entire request from client.  If request is a POST, read body of data
 * and save in file referenced by the "postfile" env variable.
 *
 * @return          0 on success, -1 on failure
 * ---------------------------------------------------------------------- */
static int parse_request(HTTP_Connection_T *http_connection, HTTP_Request_T * req)
{
    int cnt = 0;
    int rc = 0;

    char *szPost = NULL;
    int nAsk;

    int length;

    char *content_type;
    char *content_length;

    const char *multi_P = "multipart/form-data";
    int  multi_len = strlen(multi_P);
    int  i;

#ifdef DEBUG_PRINTF
        printf("parse_request\n");
#endif /* DEBUG_PRINTF */

#if (SYS_CPNT_CLUSTER == TRUE)
    /* since the parser will remove some character which to judge the end of header,
     * so using header_in_raw to keep the original head content
     */
    strncpy(req->header_in_raw,req->header_in,MAX_HEADER_LEN);
    req->header_in_raw[strlen(req->header_in_raw)-1] = '\r';
    strcat(req->header_in_raw, "\n\r\n");

    /* Becuase the commander can't handler a keepalive connection well.
     * So cleanup the flag when the request come from commander.
     */
    {
        CLUSTER_TYPE_EntityInfo_T   cluster_info;

        memset(&cluster_info, 0, sizeof(CLUSTER_TYPE_EntityInfo_T));
        CLUSTER_POM_GetClusterInfo(&cluster_info);
        if (cluster_info.role == CLUSTER_TYPE_ACTIVE_MEMBER)
        {
            http_connection->keepalive = 0;
        }
    }
#endif /* SYS_CPNT_CLUSTER */

    /* parse header data */
    if (parse_header(req) == HTTP_ERROR)
    {
        return (HTTP_ERROR);
    }

    /*-------------------------------------------------------------------
     * If the method is POST, there is an entity body after the header.
     * Some/All of the body may already be in req->header_in.  This was read
     * during read_header, and should be copied along with any unread data.
     * The POST header should have included a CONTENT_LENGTH, which is used
     * to determine the end of the body.  Copy the contents of the body into
     * a temporary file, then pass the temp file's name to the cgi application.
     -------------------------------------------------------------------*/

    if (req->method == M_POST ||
        req->method == M_PUT)
    {
        req->multipart = 0;
        content_type = get_env(req->envcfg, "CONTENT_TYPE");

        if (NULL != content_type)
        {
            for (i = 0; i <= ((int)strlen(content_type) - multi_len); i++)
            {
                if (memcmp(content_type + i, multi_P, multi_len) == 0)
                {
                    req->multipart = 1;
                    return HTTP_OK;
                }
            }
        }

        /* length */
        content_length = get_env(req->envcfg,"CONTENT_LENGTH");

        if (content_length == NULL)
        {
            req->status = R_ERROR;
            return (HTTP_ERROR);
        }

        if (content_length != NULL)
        {
            length = atoi(content_length);
        }

        /*isiah.2003-02-20*/
        if (length <= 0)
        {
            req->status = R_ERROR;
            return (HTTP_ERROR);
        }

        {
#define HTTP_REQ_1K_BYTES       (1024)
#define HTTP_REQ_4K_BYTES       (1024 << 2)
#define HTTP_REQ_16K_BYTES      (1024 << 4)
#define HTTP_REQ_32K_BYTES      (1024 << 5)
#define HTTP_REQ_64K_BYTES      (1024 << 6)
#define HTTP_REQ_128K_BYTES     (1024 << 7)
#define HTTP_REQ_256K_BYTES     (1024 << 8)

            UI32_T req_postsize[] = {
                HTTP_REQ_1K_BYTES,
                HTTP_REQ_4K_BYTES,
                HTTP_REQ_16K_BYTES,
                HTTP_REQ_POSTSIZE
                // HTTP_REQ_32K_BYTES,
                // HTTP_REQ_64K_BYTES,
                // HTTP_REQ_128K_BYTES,
                // HTTP_REQ_256K_BYTES
            };

            for (i = 0; i < _countof(req_postsize); ++ i)
            {
                if (length < req_postsize[i])
                {
                    break;
                }
            }

            /* Too big request data, using the max size we can
             */
            if (i == _countof(req_postsize))
            {
                i = _countof(req_postsize) - 1;
            }

            for ( ; 0 <= i ; -- i)
            {
                ASSERT(szPost == NULL);
                szPost = (char *) L_MM_Malloc(req_postsize[i], L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_PARSE_REQUEST));

                if (szPost)
                {
                    if (req_postsize[i] - 1 < length)
                    {
                        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_PARAMETER, 1,
                                       "%lu bytes of post data loss (content-length: %d, exactly size: %lu)",
                                       length - req_postsize[i], length, req_postsize[i]);

                        length = req_postsize[i] - 1;
                        /* Avoid to receive the remain data from some connection. Trun off keepalive flag,
                         * The remain data will be discard.
                         */
                        http_connection->keepalive = 0;
                    }
                    break;
                }

            }

            if (szPost == NULL)
            {
                HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_PARAMETER, 1,
                               "Out of memory for post data");

                req->status = R_ERROR;
                return (HTTP_ERROR);
            }
        }

        /* part of the body may already be in req->header_in */
        cnt = req->body_cnt;
        if(cnt > length)
        {
            cnt = length;
        }

        /* copy to post-text */
        memcpy (szPost, &req->header_in[req->body_index], cnt);

        /* more of body in socket (read up to "length" only) */
        /* IE gives "\r\n" after "length".  Discard. */
        while (cnt < length)
        {
            nAsk = length - cnt;
            if (nAsk > MAX_READ)
            {
                nAsk = MAX_READ;
            }
            if (cnt + nAsk > length)
            {
                nAsk = length - cnt;
            }

            rc = HTTP_UTIL_Read(http_connection, req->fd, szPost + cnt, nAsk, 0);

            if ((rc == HTTP_ERROR) || (rc < 0))
            {
                req->status = R_BAD_REQUEST;
                L_MM_Free(szPost);
                return(HTTP_ERROR);
            }

            /* update index */
            cnt += rc;
        } /* end while(cnt < length)  */

        /* end of string (cnt = length) */
        szPost[cnt] = '\0';

        /* set to query string */
        set_env(req->envcfg, "BODY", szPost);

        L_MM_Free(szPost);

    }/* endif M_POST */

    return(HTTP_OK);
}

/**----------------------------------------------------------------------
 * Parse header and set appropriate env variables.
 *
 * @return          0 on success, -1 on failure
 * ---------------------------------------------------------------------- */
static int parse_header(HTTP_Request_T * req)
{
    char *name;
    char *value;
    char *field;
    char *last_field;
    int i;
    int field_done;
    int header_done;


#ifdef DEBUG_PRINTF
        printf("parse_header\n");
#endif /* DEBUG_PRINTF */

    /* the null-terminated raw header data is now in req->header */
    /* must be at least "GET ?<CR><LF><CR><LF>" or 9 chars (? = min. URI) */
    if (req->header_end_idx < 9)
    {
        req->status = R_BAD_REQUEST;
        return(HTTP_ERROR);
    }
    /* we got at least 9 bytes here */

    /* Parse the Request-Line */

    if (parse_method(req) == HTTP_ERROR)
    {
        return(HTTP_ERROR);
    }

    if (parse_uri(req) == HTTP_ERROR)
    {
        return(HTTP_ERROR);
    }

    /* Parse the General-Header */
    /* Parse the Request-Header */

    /* req->header_idx now points to version of HTTP in header_in */
    /* parse version */
    if (! strncmp(&req->header_in[req->header_idx], "HTTP/", 5))
    {
        char ch;

        req->header_idx += 5;

        ch = req->header_in[req->header_idx ++];
        if (ch < '1' || ch > '9') {
            return(HTTP_ERROR);
        }

        req->major_version = ch - '0';

        if (req->header_in[req->header_idx ++] != '.')
        {
            return(HTTP_ERROR);
        }
        ch = req->header_in[req->header_idx ++];
        if (ch < '0' || ch > '9') {
            return(HTTP_ERROR);
        }

        req->minor_version = ch - '0';
    }

    field = req->header_in;
    do
    {
        if (req->header_in[req->header_idx++] == '\n')
        {
            req->header_in[req->header_idx - 1] = '\0';
            break;
        }
    } while (req->header_idx < req->header_end_idx);

    /* req->header_idx is index to char past request-line in header_in */

    last_field = field;

    if (field[strlen(field) - 1] == '\r')
        field[strlen(field) - 1] = '\0';

    /* don't do anything with 1st field, this was already parsed above */
    header_done = 0;
    do
    {
        field_done = 0;
        if (req->header_idx >= req->header_end_idx)
            break;

        field = &req->header_in[req->header_idx];

        while (1)
        {
            if (req->header_in[req->header_idx++] == '\n')
            {
                req->header_in[req->header_idx - 1] = '\0';
                break;
            }

            if (req->header_idx >= req->header_end_idx)
            {
                field = NULL;
                header_done = 1;
                break;
            }
        };

        if (field == NULL)
            break;

        if (field[strlen(field) - 1] == '\r')
            field[strlen(field) - 1] = '\0';

        /* check for field found, and that field found is
         * not same as last field found
         */
        if((strlen(field)) && (field != last_field))
        {
            last_field = field;

            /* seperate name & value from field and set ptrs */
            name = field;
            for(i = 0; i < strlen(field); i++)
            {
                if(field[i] == ':')
                {
                    char *end;

                    value = &field[i+1];
                    field[i] = '\0';

                    end = &value[strlen(value)];

                    /* Trim space of value
                     */
                    for (; *value && *value == ' '; ++ value);
                    for (; value < end && *(--end) == ' ';) { *end = '\0'; }

                    break;
                }
            }

            /* Skip header if value is a blank string
             */
            if (*value == '\0')
            {
                continue;
            }

            /* Convert name to upper case and replace '-' with '_' */
            for(i = 0; i < strlen(name); i++)
            {
                if(name[i] == '-')
                    name[i] = '_';
                else
                    name[i] = toupper(name[i]);
            }

            /* Compare name with set of standard env fields; if found set env */
            for(i = 0; http_standard_env[i] != 0; i++)
            {
                if(!strcmp(name, http_standard_env[i]))
                {
                    set_env(req->envcfg, name, value);
                    field_done = 1;
                    break;
                }

                /* for reload htm referer */
                if (strcmp(name, "REFERER") == 0)
                {
                    set_env(req->envcfg, name, value);
                    field_done = 1;
                    break;
                }
            }
        }
        else
            header_done = 1;
    } while (!header_done);

    return(HTTP_OK);
}

/**----------------------------------------------------------------------
 * Determine method type
 *
 * @return          0 on success, -1 on failure
 * ---------------------------------------------------------------------- */
static int parse_method(HTTP_Request_T * req)
{

#ifdef DEBUG_PRINTF
        printf("parse_method\n");
#endif /* DEBUG_PRINTF */

    if (! strncmp(req->header_in, "GET ", 4))
    {
        req->method = M_GET;
        req->header_idx = 4;
        set_env(req->envcfg, "METHOD", "GET");
    }
    else if (! strncmp(req->header_in, "HEAD ", 5))
    {
        req->method = M_HEAD;
        req->header_idx = 5;
        set_env(req->envcfg, "METHOD", "HEAD");
    }
    else if (! strncmp(req->header_in, "POST ", 5))
    {
        req->method = M_POST;
        req->header_idx = 5;
        set_env(req->envcfg, "METHOD", "POST");
    }
    else if (! strncmp(req->header_in, "PUT ", 4))
    {
        req->method = M_PUT;
        req->header_idx = 4;
        set_env(req->envcfg, "METHOD", "PUT");
    }
    else if (! strncmp(req->header_in, "DELETE ", 7))
    {
        req->method = M_DELETE;
        req->header_idx = 7;
        set_env(req->envcfg, "METHOD", "DELETE");
    }
    else
    {   /* not a valid method */
        req->status = R_BAD_REQUEST;
        return(HTTP_ERROR);
    }

    return (HTTP_OK);
}

/**----------------------------------------------------------------------
 * copy the uri from header_in to request_uri. update header_idx.
 * update http_simple.
 *
 * @return          0 on success, -1 on failure
 * ---------------------------------------------------------------------- */
static int parse_uri(HTTP_Request_T * req)
{
int uri_idx = 0;

#ifdef DEBUG_PRINTF
        printf("parse_uri\n");
#endif /* DEBUG_PRINTF */

    while ((req->header_in[req->header_idx] != '\0') &&
           (req->header_in[req->header_idx] != ' '))
    {
        req->request_uri[uri_idx] = req->header_in[req->header_idx];

        uri_idx++;          /* destination */
        req->header_idx++;      /* source */

        if (uri_idx >= MAX_URI_LEN)
        {
        /* URI is too long */
            req->status = R_BAD_REQUEST;
            return(HTTP_ERROR);
        }
    }

    /* null-terminate the uri string */
    req->request_uri[uri_idx] = '\0';

    /* if at end of GET request header then it must be
     * a simple request
     */
    if (req->header_in[req->header_idx] == '\0')
    {
        if (req->method == M_GET)
        {
            req->http_simple = TRUE;
            cleanup_uri(req->request_uri, req);
            return(HTTP_OK);
        }
        else
        {  /* malformed Full-Request header */
            req->status = R_BAD_REQUEST;
            return(HTTP_ERROR);
        }
    }

    req->http_simple = FALSE;

    /* now move index to HTTP-Version in header since we have one*/
    req->header_idx++;
    cleanup_uri(req->request_uri, req);

    set_env(req->envcfg, "URI", req->request_uri);

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_CONNECTION, 1,
        "URI: %s", req->request_uri);

    set_path(req);

    return(HTTP_OK);
}

/**----------------------------------------------------------------------
 * map uri to path and set the path to envcfg.
 * ---------------------------------------------------------------------- */
static void set_path(HTTP_Request_T * req)
{
    char *file_path;

    ASSERT(req);
    ASSERT(req->request_uri);

    file_path = HTTP_MGR_MapUriToPath(req->request_uri);

    if (file_path)
    {
        set_env(req->envcfg, "PATH_INFO", file_path);

        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_CONNECTION, 1,
            "PATH_INFO: %s", file_path);

        free(file_path);
    }
}

/**----------------------------------------------------------------------
 * remove any ".." from a request uri for security.
 * Set env variable QUERY_STRING if a ? is found in the uri.
 * ---------------------------------------------------------------------- */
static void cleanup_uri(char * request_uri, HTTP_Request_T *req)
{
    char *buffer;
    int i = 0;
    int src_idx = 0;
    int dest_idx = 0;
    int max_src_idx = strlen(request_uri) - 1;
    BOOL_T is_rest_url = FALSE;

#if (SYS_CPNT_CLUSTER == TRUE)
#define CGI_TYPE_CLUSTER_ID_DIGIT_LENGTH    2

    /* id range 1~16, and add '\0' to tail
     */
    char str_cluster_id[CGI_TYPE_CLUSTER_ID_DIGIT_LENGTH+1] = {0};
#endif /* SYS_CPNT_CLUSTER */


#ifdef DEBUG_PRINTF
        printf("cleanup_uri\n");
#endif /* DEBUG_PRINTF */

    if ((buffer = (char *) L_MM_Malloc(MAX_URI_LEN, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_CLEANUP_URI))) == 0)
    {
        return;
    }

    if (strncmp(request_uri, "/api/", strlen("/api/")) == 0)
    {
        is_rest_url = TRUE;
    }

    /* strip off first '.' of a '..', resultant path may be bad  */

    /* loop until we find a '?' or until end of string */
    while ((src_idx <= max_src_idx) && (request_uri[src_idx] != '?'))
    {
        if ((request_uri[src_idx] == '.') &&
            (request_uri[src_idx + 1] == '.'))
        {
            src_idx++;
        }
        else
        {
#if (SYS_CPNT_CLUSTER == TRUE)
            /* analyze the virtual path and save the cluster_id environment */
            if ( (FALSE == is_rest_url) &&
                ( strncmp(&request_uri[src_idx], "/member", 7) == 0 ) )
            {
                UI16_T cluster_id_index=0;
                src_idx = src_idx + 7; /* shift to the positon of cluster id */
                do
                {
                    /* if cluster id is large than two digits , must regard as
                     * error and break loop
                     */
                    if (cluster_id_index == CGI_TYPE_CLUSTER_ID_DIGIT_LENGTH)
                        break;
                    str_cluster_id[cluster_id_index] = request_uri[src_idx];
                    cluster_id_index++;
                    src_idx++; /* jump to next character '/' */
                } while (request_uri[src_idx] != '/');

                str_cluster_id[cluster_id_index]= '\0';
                set_env(req->envcfg, "REMOTE_CLUSTER_ID", str_cluster_id);
            }
#endif /* SYS_CPNT_CLUSTER */
            request_uri[dest_idx] = request_uri[src_idx];
            dest_idx++;
            src_idx++;
        }
    }

    request_uri[dest_idx] = '\0';
    src_idx++;

    /* copy parameter list unaltered */

    while (src_idx <= max_src_idx)
    {
        buffer[i++] = request_uri[src_idx++];
    }
    buffer[i] = 0;  /* NULL terminate */

    if(i)
    {
        set_env(req->envcfg,"QUERY_STRING", buffer);
    }
    L_MM_Free(buffer);
}

/**----------------------------------------------------------------------
 * Read date from socket until find end of header or error occurs.
 *
 * @return          0 on success, -1 on failure
 * ---------------------------------------------------------------------- */
static int http_request_read_headers(HTTP_Connection_T *http_connection)
{
    int index;
    int prev_char;
    int rv;
    int len;

    HTTP_Request_T *req = http_connection->req;

#ifdef DEBUG_PRINTF
    printf("http_request_read_headers\n");
#endif /* DEBUG_PRINTF */

    int read_byte = (sizeof(req->header_in) - req->header_length);

    len = HTTP_UTIL_Read(http_connection, req->fd,
                         &req->header_in[req->header_length],
                         read_byte,
                         0);
    if (len <= 0)
    {
        err_end(E_RECV, req);
        req->status = R_ERROR;
        return (HTTP_ERROR);
    }

    req->header_length += len;

    ASSERT(req->header_length <= sizeof(req->header_in));

    /* Low performance this time
     */
    index = 0;
    prev_char = NORMAL;

    rv = check_for_end_of_header(req, &index, &prev_char);

    if (rv)
    {
        ASSERT(index == req->header_length);
        return HTTP_AGAIN;
    }

    ASSERT(index <= req->header_length);

    /* the header is terminated properly   */

    /* index points to first char of body */

    req->body_index = index;
    req->body_cnt = req->header_length - index;


    req->header_end_idx = strlen(req->header_in);       /* adjust index to end of header */

    ASSERT(req->header_end_idx < req->body_index);
    ASSERT(req->header_end_idx < req->header_length);

    req->header_in[req->header_end_idx - 1] = '\n';    /* leave a single LF at end for use with parse_header */

    return(HTTP_OK);
}

/**----------------------------------------------------------------------
 * search through data to determine if we recv'd a full header
 * a complete header is terminated by a CRLFCRLF or CRCR
 * we will terminate the header leaving CRNULL
 *
 * @return          0 on success, -1 on failure
 * ---------------------------------------------------------------------- */
static int check_for_end_of_header(HTTP_Request_T *req, int *index, int *prev_char)
{
#ifdef DEBUG_PRINTF
        printf("check_for_end_of_header\n");
#endif /* DEBUG_PRINTF */

    /* search through data to determine if we recv'd a full header
     * a complete header is terminated by a CRLFCRLF or CRCR
     * we will terminate the header leaving CRNULL
     */

    while (*index < req->header_length)
    {
        if (req->header_in[*index] == CR)
        {
            if (*prev_char == CR)
            {
                *prev_char = CRCR;
                req->header_in[*index] = 0; /* NULL terminate header */
                (*index)++;
                return 0;           /* found end of header */
            }
            else if (*prev_char == CRLF)
            {
                *prev_char = CRLFCR;
                (*index)++;
            }
            else
            {
                *prev_char = CR;
                (*index)++;
            }
        }
        else if (req->header_in[*index] == LF)
        {
            if (*prev_char == CR)
            {
                *prev_char = CRLF;
                (*index)++;
            }
            else if (*prev_char == CRLFCR)
            {
                *prev_char = CRLFCRLF;
                req->header_in[(*index) - 2] = 0;     /* NULL terminate header */
                (*index)++;
                return 0;               /* found end of header */
            }
            else
            {
                *prev_char = NORMAL;
                (*index)++;
            }
        }
        else
        {
            *prev_char = NORMAL;
            (*index)++;
        }
    }
    return 1;                       /* did not find end of header */
}

/**----------------------------------------------------------------------
 * Send an error reply message to client
 *
 * @param  req
 * @return      0 on success, -1 on failure
 * ---------------------------------------------------------------------- */
static int send_error_reply(HTTP_Connection_T *http_connection, HTTP_Request_T * req)
{
    char *buffer;

#ifdef DEBUG_PRINTF
        printf("send_error_reply\n");
#endif /* DEBUG_PRINTF */

    if ((buffer = (char *) L_MM_Malloc(MAX_ERROR_HEADER_LEN, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_SEND_ERROR_REPLY))) == 0)
    {
        return (-1);
    }

    /* generate header and send relevant error message */
    switch(req->status)
    {
        case R_BAD_REQUEST:
        {
            strcpy(buffer, "HTTP/1.0 400 Bad Request\r\n");
            strcat(buffer, "Content-Type: text/html\r\n\r\n");
            strcat(buffer, "<title>400 Bad Request</title><body>400 Bad Request</body>");
            break;
        }

        case R_FORBIDDEN:
        {
            strcpy(buffer, "HTTP/1.0 403 Forbidden\n");
            strcat(buffer, "Content-Type: text/html\n\n");
            strcat(buffer, "<title>403 Forbidden</title><body>403 Forbidden</body>");
            break;
        }

        case R_METHOD_NA:
        {
            strcpy(buffer, "HTTP/1.0 405 Method Not Allowed\n");
            strcat(buffer, "Content-Type: text/html\n\n");
            strcat(buffer, "<title>405 Method Not Allowed</title><body>405 Method Not Allowed</body>");
            break;
        }

        case R_NOT_FOUND:
        {
            strcpy(buffer, "HTTP/1.0 404 Not Found\r\n");
            strcat(buffer, "Content-Type: text/html\r\n\r\n");
            strcat(buffer, "<title>404 Not found</title><body>404 Not Found</body>");
            break;
        }

        case R_NOT_IMP:
        {
#if 0 /* ignored, Zhong Qiyao, 1998-12-23 */
            strcpy(buffer, "HTTP/1.0 500 Internal Server Error\n");
            strcat(buffer, "Content-Type: text/html\n\n");
            strcat(buffer, "<title>500 Internal Server Error</title><body>500 Internal Server Error</body>");
#endif
            buffer[0] = '\0'; /* empty message */

            break;
        }

        case R_ERROR:
        {
#if 0 /* ignored, Zhong Qiyao, 1998-12-23 */
            strcpy(buffer, "HTTP/1.0 599 Unknown Error\n");
            strcat(buffer, "Content-Type: text/html\n\n");
            strcat(buffer, "<title>599 Unknown Error</title><body>599 Unknown Error</body>");
#endif
            buffer[0] = '\0'; /* empty message */

            break;
        }

        case R_SILENT:
        {
            break;
        }

#if (SYS_CPNT_WEBAUTH == TRUE)
        case R_MOVED_TEMP:
        {
            UI32_T dut_addr;
            UI32_T peer_addr;
            UI16_T webauth_type = WEBAUTH_BLACK_HOST;
            char url_ar[WEBAUTH_TYPE_MAX_URL_LENGTH + 1] = { 0 };
            char ip_str[18] = { 0 };
            char ori_host_ar[WEBAUTH_TYPE_MAX_URL_LENGTH + 1] = { 0 };
            char *buf_p;

            ASSERT(get_env(req->envcfg, "WEBAUTH_TYPE") != NULL);

            if (get_env(req->envcfg, "WEBAUTH_TYPE") != NULL)
                webauth_type = atoi(get_env(req->envcfg, "WEBAUTH_TYPE"));

            /* process original url */
            if (get_env(req->envcfg, "URI") != NULL)
            {
                /* 120-7(http://)-15(host_ip)=98, must count submit
                 * part(fail folder path)
                 */
                if (strlen(get_env(req->envcfg, "URI")) <= 77)
                {
                    strcpy(ori_host_ar, "http://");
                    buf_p = get_env(req->envcfg, "HOST");
                    /* because host with ip will have one space */
                    while (*buf_p == ' ')
                        buf_p++;

                    strcat(ori_host_ar, buf_p);
                    strcat(ori_host_ar, get_env(req->envcfg, "URI"));
                }
            }

            ASSERT(get_env(req->envcfg, "IP_ADDR") != NULL);

            if (get_env(req->envcfg, "IP_ADDR") == NULL)
            {
                cgi_response_end(http_connection);
                L_MM_Free(buffer);
                return(HTTP_ERROR);
            }

            L_INET_Aton((UI8_T *)get_env(req->envcfg, "IP_ADDR"), &peer_addr);
            if (FALSE == get_dut_address(peer_addr, &dut_addr))
            {
                cgi_response_end(http_connection);

                /* Discard request if get switch IP address failed.
                 */
                L_MM_Free(buffer);
                return(HTTP_ERROR);
            }

            L_INET_Ntoa(dut_addr, (UI8_T *)ip_str);

            switch (webauth_type)
            {
                case WEBAUTH_REDIRECT:
                    strcpy(url_ar, "/webauth/login.htm?oriurl=");
                    strcat(url_ar, ori_host_ar);
                    break;
                case WEBAUTH_TABLE_FULL:
                    strcpy(url_ar, "/webauth/login_full.htm");
                    break;
                case WEBAUTH_BLACK_HOST:
                    strcpy(url_ar, "/webauth/login_fail_held.htm");
                    break;
            }
            strcpy(buffer, "HTTP/1.0 302 Found\r\n");
            strcat(buffer, "Location: HTTP://");
            strcat(buffer, ip_str);

            {
                UI32_T http_port = HTTP_OM_Get_Http_Port();
                if (http_port != 80)
                {
                    char _port_buf[20];
                    snprintf(_port_buf, sizeof(_port_buf), ":%lu", http_port);
                    _port_buf[sizeof(_port_buf) - 1] = '\0';
                    strcat(buffer, _port_buf);
                }
            }

            strcat(buffer, url_ar);
            strcat(buffer, "\r\n\r\n");
            strcat(buffer, "Content-Type: text/html\r\n\r\n");
            strcat(buffer, "<html><title>302 found</title><body>move temp</body></html>");

            break;
        }
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

        default:
        {
#if 0 /* ignored, Zhong Qiyao, 1998-12-23 */
            strcpy(buffer, "HTTP/1.0 599 Unknown Error\n");
            strcat(buffer, "Content-Type: text/html\n\n");
            strcat(buffer, "<title>599 Unknown Error</title><body>599 Unknown Error</body>");
#endif
            buffer[0] = '\0'; /* empty message */

            break;
        }
    }

    if (HTTP_UTIL_Write(http_connection, req->fd, buffer, strlen(buffer), 0) == -1)
    {
        L_MM_Free(buffer);
        return(HTTP_ERROR);
    }

    //
    // FIXME: !!
    //
    cgi_response_end(http_connection);

    L_MM_Free(buffer);

    return(HTTP_OK);
}

/* LOCAL SUBPROGRAM BODIES
 */
#if (SYS_CPNT_HTTPS == TRUE)


/**----------------------------------------------------------------------
 * Connect SSL to the accepted socket.
 *
 * @param  req      request structure
 * @param  port_num port number
 * @return          return-value-1 - return value (1) to indicate success
 *                  and (0) to indicate failure.
 * ---------------------------------------------------------------------- */
static UI32_T HTTP_Handshake(HTTP_Request_T *req,int port_num)
{
    SSL_CTX *ssl_ctx;
    char buf[4096]; // TODO: Why need this larger buffer ??
    char *str = NULL;
    int rc;
    int http_port;
    UI32_T secure_port;
    BOOL_T is_secure_port_changed = FALSE;

    http_port = HTTP_MGR_Get_Http_Port();
    HTTP_MGR_Get_Secure_Port(&secure_port);

    if (secure_port != HTTP_DEFAULT_SECURE_PORT_NUMBER)
    {
        is_secure_port_changed = TRUE;
    }

    if (port_num == http_port)
    {
        return (HTTP_HANDSKAHE_SUCCESS);
    }
    else if (port_num == secure_port)
    {
        if (req->ssl) return HTTP_HANDSKAHE_SUCCESS;

        /*HTTP_MGR_Seed_SSL_Rand();*/
        ssl_ctx = (SSL_CTX *)HTTP_MGR_Get_SSL_CTX();
        if (ssl_ctx == NULL)
        {
            return(HTTP_HANDSKAHE_FAILURE);
        }

        req->ssl = SSL_new(ssl_ctx);

        if (req->ssl == NULL)
        {
            return(HTTP_HANDSKAHE_FAILURE);
        }
        /*======================*/
        ERR_load_BIO_strings();
        /*======================*/
        SSL_clear(req->ssl);
        //Set session id context.//??Maybe not need.Will be used when server verifys client certificate.
        SSL_set_fd(req->ssl, req->fd);

        if ((rc = SSL_accept(req->ssl)) <= 0)
        {
            UI32_T  error_code;

            error_code = ERR_GET_REASON(ERR_peek_error());
            if (error_code == SSL_R_HTTP_REQUEST)
            {
                // TODO: Split into different function for recv (another event)
                recv(req->fd, buf, sizeof(buf) - 1, 0);//skip the remaining bytes of the request line
                buf[sizeof(buf) - 1] = '\0';

                if (HTTP_Get_Host_Ip_From_Request(buf, &str, secure_port, is_secure_port_changed) == TRUE)
                {
                    send(req->fd, str, strlen(str), 0);
                    L_MM_Free(str);
                }

                return HTTP_HANDSKAHE_FAILURE;
            }
            else if (error_code == 0)
            {
                if (rc == EWOULDBLOCK /* -12 */)
                {
                    return HTTP_HANDSKAHE_NO_PACKET;
                }
                else
                {
                    return HTTP_HANDSKAHE_BROKEN;
                }
            }
            else if (error_code == SSL_R_SSL_HANDSHAKE_FAILURE)
            {
                return HTTP_HANDSKAHE_NO_SECOND_HELLO;
            }

            //handshake error.
            EH_MGR_Handle_Exception(SYS_MODULE_HTTP, HTTP_Handshake_FUNC_NO, EH_TYPE_MSG_SSL_HANDSHAKE_FAILED, SYSLOG_LEVEL_INFO);
            return(HTTP_HANDSKAHE_FAILURE);
        }
        else
        {
            return(HTTP_HANDSKAHE_SUCCESS);
        }
    }
    else
    {
        ASSERT(0);
        return(HTTP_HANDSKAHE_PORT_FAILURE);
    }
}

/**----------------------------------------------------------------------
 * Auto-redirect https homepage when received http request.
 *
 * @param  src              http request string.
 * @param  dest
 * @param  secure_port
 * @param  is_port_changed
 * @return                  return-value-1 - Auto-redirect command string.
 * ---------------------------------------------------------------------- */
static BOOL_T HTTP_Get_Host_Ip_From_Request(char *src, char **dest, int secure_port, BOOL_T is_port_changed)
{
#define TMP_MESSAGE_BEGIN "<html><body><H2>Instead use the HTTPS scheme to access this URL,please.<br>Or,<br>wait 5 seconds,auto redirect to https://"
#define TMP_MESSAGE_END "/</H2>"
#define TMP_BUF_BEGIN "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"5;url=https://"
#define TMP_BUF_END "\"></body></html>"

    char *result;
    char *srcP;

    int srcLen;
    int i, hostIdx;
    char len[10];

    srcP = src;
    srcLen = strlen(src);
    for (i = 0; i<srcLen; src++, i++)
    {
        if (strncmp(src, "Host:", 5) == 0)
        {
            i += 5;
            src = src + 5;
            while (*src == ' ')
            {
                i++;
                src++;
            }
            hostIdx = i;
            while ((*src != '\n') && (*src != '\r') && (*src != ':'))
            {
                i++;
                src++;
            }
            src = srcP + hostIdx;
            if (is_port_changed)
            {
                sprintf(len, ":%d", secure_port);
#ifdef DEBUG_PRINTF
                printf("len=%s,%d",len,strlen(len));
#endif /* DEBUG_PRINTF */
                result = (char *)L_MM_Malloc(strlen(TMP_MESSAGE_BEGIN) + (i - hostIdx) + strlen(TMP_MESSAGE_END) + strlen(TMP_BUF_BEGIN) + (i - hostIdx) + strlen(TMP_BUF_END) + 1 + 12, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_HTTP_GET_HOST_IP_FROM_REQUEST));
            }
            else
            {
                memset(len, 0, 10);
                result = (char *)L_MM_Malloc(strlen(TMP_MESSAGE_BEGIN) + (i - hostIdx) + strlen(TMP_MESSAGE_END) + strlen(TMP_BUF_BEGIN) + (i - hostIdx) + strlen(TMP_BUF_END) + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_HTTP_GET_HOST_IP_FROM_REQUEST));
            }
            if (result == NULL)
            {
                return FALSE;
            }
            strcpy(result, TMP_MESSAGE_BEGIN);
            strncat(result, src, (i - hostIdx));
            strncat(result, len, strlen(len));
            strcat(result, TMP_MESSAGE_END);
            strcat(result, TMP_BUF_BEGIN);
            //src = srcP + hostIdx;
            strncat(result, src, (i - hostIdx));
            strncat(result, len, strlen(len));
            strcat(result, TMP_BUF_END);
            *dest = result;
            return TRUE;
        }
    }
    return FALSE;
}
#endif /* SYS_CPNT_HTTPS */

/**----------------------------------------------------------------------
 * get port number from sockaddr
 *
 * @return  -1 if failed
 * ---------------------------------------------------------------------- */
static int HTTP_GetSockPort(struct sockaddr *sa)
{
    switch (sa->sa_family)
    {
    case AF_INET:
        {
            struct sockaddr_in  *sin = (struct sockaddr_in *) sa;

            return ntohs(sin->sin_port);
        }

#if (SYS_CPNT_IPV6 == TRUE)
    case AF_INET6:
        {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;

            return ntohs(sin6->sin6_port);
        }
#endif
    }

    return(-1);     /* ??? */
}

#if (SYS_CPNT_CLUSTER == TRUE)
static BOOL_T http_relaying(HTTP_Connection_T *http_connection)
{
    int    read,remaining, send_len, retry_count_send;
    char   *p_receive_buf;
    char   *p_receive_buf_tmp;

    retry_count_send = 0;
    p_receive_buf = (char *) L_MM_Malloc(HTTP_CLUSTER_MAX_RELAY_BUF_SIZE, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_HTTP_PROCESS_REQUEST));
    if(NULL == p_receive_buf)
    {
        return FALSE;
    }

    memset(p_receive_buf, 0, HTTP_CLUSTER_MAX_RELAY_BUF_SIZE);

    while (1)
    {
        /* receive data from member
         */
#ifndef OS_WINDOWS
        read = recv(http_connection->socket_relay_id, p_receive_buf, HTTP_CLUSTER_MAX_RELAY_BUF_SIZE, MSG_DONTWAIT);
#else
        read = recv(http_connection->socket_relay_id, p_receive_buf, HTTP_CLUSTER_MAX_RELAY_BUF_SIZE, 0);
#endif /* OS_WINDOWS */

        if(read <= 0)
        {
            if(EWOULDBLOCK == errno)
            {
                if (read == 0)
                {
                    L_MM_Free(p_receive_buf);
                    return TRUE;
                }

                if(retry_count_send <= HTTP_CLUSTER_MAX_RETRY_TIME)
                {
                    retry_count_send ++;
                    SYSFUN_Sleep(10);
                    continue;
                }
                else
                {
                    L_MM_Free(p_receive_buf);
                    return FALSE;
                }
            }
            else
            {
                L_MM_Free (p_receive_buf);
                return TRUE;
            }
        }
        else
        {
            retry_count_send = 0;
            p_receive_buf_tmp = p_receive_buf;
            remaining = read;

            while(remaining > 0)
            {
#ifndef OS_WINDOWS
                send_len = send(http_connection->fds[HTTP_CONN_FD_NET], p_receive_buf_tmp, remaining, MSG_DONTWAIT);
#else
                send_len = send(http_connection->fds[HTTP_CONN_FD_NET], p_receive_buf_tmp, remaining, 0);
#endif /* OS_WINDOWS */

                if(send_len <= 0)
                {
                    if((errno == EWOULDBLOCK) && retry_count_send <= HTTP_CLUSTER_MAX_RETRY_TIME)
                    {
                        retry_count_send++;
                        SYSFUN_Sleep(10);
                        continue;
                    }
                    /* Warning,the date received from member is timeout
                     * while sending to peer
                     */
                    else
                    {
#ifdef DEBUG_PRINTF
                        printf("Warning,http-relay to peer is timeout\n");
#endif /* DEBUG_PRINTF */
                        L_MM_Free(p_receive_buf);
                        return TRUE;
                    }
                }
                remaining-=send_len;
                p_receive_buf_tmp += send_len;
            }
            retry_count_send = 0;
        }
        SYSFUN_Sleep(10);
    }
}

static BOOL_T http_relay_action(HTTP_Request_T * req, unsigned int cluster_id, HTTP_Connection_T *http_connection)
{
    struct timeval      timeout;
    struct sockaddr_in  sin;
    fd_set              write_fds;
    int                 sockfd, on = 1, off = 0;
    int                 remaining, sent, read, ret, retry_count_recv,retry_count_send;
    char               *ptr;
    UI32_T              i_member_ip;
    UI8_T               member_ip[4];
    int                 f_status;

    /* 1. create socket
     */
    if ((sockfd = socket(AF_INET , SOCK_STREAM, 0)) < 0)
    {
#ifdef DEBUG_PRINTF
        printf("aborted http_relay_action on socket()\n");
#endif
        s_close(sockfd);
        return TRUE;
    }

    if (FALSE == CLUSTER_POM_MemberIdToIp(cluster_id,member_ip))
    {
        s_close(sockfd);
        return TRUE;
    }

    memcpy(&i_member_ip,member_ip,sizeof(UI32_T));
    memset(&sin, 0, sizeof (sin));
    sin.sin_addr.s_addr = htonl(i_member_ip);
    sin.sin_family = AF_INET;

    /* FIXME: Security issue (design or code problem ? !)
     *        will proxy a HTTPS request as HTTP ! !
     */
    sin.sin_port = htons(HTTP_MGR_Get_Cluster_Port()); /* should get from member */

#ifndef OS_WINDOWS
    /* set the non-blocking mode of socket connect to member. */
    f_status = fcntl(sockfd, F_GETFL);
    fcntl(sockfd, F_SETFL,f_status | O_NONBLOCK);
#endif /* OS_WINDOWS */

    /* 2. connect socket
     */
    if ( ( ret = connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) ) < 0 )
    {
        if ( errno != EINPROGRESS )
        {
#ifdef DEBUG_PRINTF
            printf("aborted http_relay_action on connent(%d)\n",ret);
#endif
            s_close(sockfd);
            return TRUE;
        }
    }

    /* prepare select */
    FD_ZERO(&write_fds);
    FD_SET(sockfd, &write_fds);
    timeout.tv_sec  = 1;     /*  no.  of seconds  */
    timeout.tv_usec = 0;     /*  no.  of micro seconds  */

    /* check connect() if success */

    ret = select( sockfd + 1, 0, &write_fds, 0, &timeout );

    if (ret <= 0 )
    {
        if ( ret == 0 )
        {
#ifdef DEBUG_PRINTF
            printf("aborted http_relay_action on connent() -- timeout \n");
#endif
        }
        else
        {
#ifdef DEBUG_PRINTF
            printf("aborted http_relay_action on connent() -- failed(%d) \n",ret);
#endif
        }
        s_close(sockfd);
        return TRUE;
    }

    ASSERT(http_connection->socket_relay_id < 0);
    http_connection->socket_relay_id = sockfd;
    http_connection->is_relaying = TRUE;
    retry_count_send = 0;
    sent        = 0;
    ptr         = req->header_in_raw;
    remaining   = strlen(ptr);

    /* send header */
    while (remaining > 0)
    {
        sent = send(sockfd, ptr, remaining, 0);
        if (sent <= 0)
        {
            /* block happened, retry */
            if ( ( EWOULDBLOCK == errno ) && ( retry_count_send <= HTTP_CLUSTER_MAX_RETRY_TIME) )
            {
                retry_count_send++;
                SYSFUN_Sleep(1);
                continue;
            }
            s_close(sockfd);
            return TRUE;
        }
        retry_count_send = 0;
        remaining -= sent;
        ptr += sent;
    }

    /* send body */
    if (req->method == M_POST)
    {
        retry_count_send  = 0;
        sent         = 0;
        ptr          = get_env(req->envcfg, "QUERY_STRING");
        remaining    = strlen(ptr);

        while (remaining > 0)
        {
#ifndef OS_WINDOWS
            sent = send(sockfd, ptr, remaining, MSG_DONTWAIT);
#else
            sent = send(sockfd, ptr, remaining, 0);
#endif /* OS_WINDOWS */
            if (sent <= 0)
            {
                /* block happened, retry*/
                if ( ( EWOULDBLOCK == errno ) && ( retry_count_send <= HTTP_CLUSTER_MAX_RETRY_TIME ) )
                {
                    retry_count_send++;
                    SYSFUN_Sleep(1);
                    continue;
                }

#ifdef DEBUG_PRINTF
                printf("aborted http_connect on send() body (%d)\n", retry_count_send);
#endif
                s_close(sockfd);
                return TRUE;
            }
            retry_count_send = 0;
            remaining -= sent;
            ptr += sent;

        }
    }

    /* 4. receive
     */
    return(http_relaying(http_connection));
}
#endif /* SYS_CPNT_CLUSTER */
