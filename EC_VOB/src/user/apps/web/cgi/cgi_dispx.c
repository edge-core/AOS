/* ----------------------------------------------------------------------
 * FILE NAME: cgi\cg_dispx.c

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by project manager.

 * Module for Fileless CGI.
 * This is the file for dispatch (in conjuction with cg_disp.c).

 * CLASSES AND FUNCTIONS:

 * Basic services:

 * HISTORY:
 * 1999-03-23 (Tue): Created by Zhong Qiyao.

 * COPYRIGHT (C) Accton Technology Corporation, 1999.
 * ---------------------------------------------------------------------- */

#include <string.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "l_mm.h"
#include "fs.h"
#include "cgi.h"
#include "cgi_auth.h"
#include "cgi_submt.h"
#include "cgi_cache.h"
#include "cgi_file.h"
#include "cgi_rest.h"

#include "buffer_mgr.h"
/* ----------------------------------------------------------------------
 * External
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * Types.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * Constants.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * Global constants.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * In-file constants.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * Forward declarations.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * cgi_disp_before: Action befort dispatching.

 * Input:
 * sock: Socket identifier.
 * auth: Authorisation level: 1=guest, 2=administrator.
 * envcfg: Environment configuration.
 * envQuery: Query parameters.

 * Return:
 * 1: Have sent screen.
 * 0: Have not sent screen.
 * -1: Error.
 * ---------------------------------------------------------------------- */

static int cgi_disp_before(HTTP_Connection_T *http_connection, int sock, int auth, envcfg_t *envcfg, envcfg_t *envQuery)
{
    int nRet;

    nRet = cgi_submit(http_connection, sock, auth, envcfg, envQuery);

    return (nRet);
}

/* ----------------------------------------------------------------------
 * cgi_disp_after: Action after dispatching.

 * Input:
 * sock: Socket identifier.
 * auth: Authorisation level: 1=guest, 2=administrator.
 * envcfg: Environment configuration.
 * envQuery: Query parameters.

 * Return:
 * 1: Have sent screen.
 * 0: Have not sent screen.
 * -1: Error.
 * ---------------------------------------------------------------------- */

static int cgi_disp_after(HTTP_Connection_T *http_connection, int sock, int auth, envcfg_t *envcfg, envcfg_t *envQuery)
{
    return 0;
}

/* ----------------------------------------------------------------------
 * cgi_dispatch: Read from flash disk

 * Input:
 * sock: Socket identifier.
 * auth: Authorisation level: 1=guest, 2=administrator.
 * envcfg: Environment configuration.
 * envQuery: Query parameters.
 * ---------------------------------------------------------------------- */
static int cgi_disp_file(HTTP_Connection_T *http_connection, int sock, int auth, envcfg_t *envcfg, envcfg_t *envQuery)
{
    return CGI_FILE_Main(http_connection, sock, auth, envcfg, envQuery);
}

/* ----------------------------------------------------------------------
 * cgi_disp_fixedWebDownload: Sends File context(runtime image or config file) via HTTP protocol.

 * Input:
 * sock: Socket identifier.
 * auth: Authorisation level: 1=guest, 2=administrator.
 * envcfg: Environment configuration.
 * envQuery: Query parameters.
 * ---------------------------------------------------------------------- */
static int cgi_disp_fixedWebDownload(HTTP_Connection_T *http_connection, int sock, int auth, envcfg_t *envcfg, envcfg_t *envQuery)
{
    char *path = get_env(envcfg, "URI");
    char *pUnit;
    char *pFile;
    char *reloadhtm;
    UI8_T *pBuf = NULL;
    UI32_T nUnit = 1;
    UI32_T read_count;
    UI32_T nRet = 0;

    // TODO: Trace this path when using an under access level
    if (auth < CGI_HTTP_DOWNLOAD_PRIVILEGE_REQUEST)
    {
        set_env(envcfg, "CHANGE_CONTENT_TYPE", "text/html\r\n");

        cgi_submit_ErrorStateByStr(http_connection, sock, envcfg, "User privileges are not enough to perform this operation.", &nRet);

        if (nRet == 1)
        {
            const char *empty_fmt = "<html><body><script></script></body></html>\n";
            const char *err_fmt = "<html><body><script>alert('%s');</script></body></html>\n";

            CGI_RESPONSE_USER_CONTEXT_PTR_T ctx;

            ctx = (CGI_RESPONSE_USER_CONTEXT_PTR_T) http_connection->res->user_ctx;

            reloadhtm = (char *) calloc(LOCATE_MEM_SIZE, 1);

            if (reloadhtm != NULL)
            {
                if (ctx->have_error)
                {
                    sprintf(reloadhtm, err_fmt, ctx->error_str);
                }
                else
                {
                    strncpy(reloadhtm, empty_fmt, LOCATE_MEM_SIZE - 1);
                    reloadhtm[LOCATE_MEM_SIZE - 1] = '\0';
                }

                cgi_SendHeader(http_connection, sock, -1, envcfg);
                cgi_SendText(http_connection, sock, (const char *)reloadhtm);
                cgi_response_end(http_connection);
                free(reloadhtm);
                reloadhtm = NULL;
            }
        }

        return nRet;
    }

    /* path="/file/xxxxxx" */
    if (0 == memcmp(path,"/file/",strlen("/file/")))
    {
        pFile = path + strlen("/file/");

        if ((pUnit = get_env (envQuery, "unit")) != NULL)
        {
            nUnit = atol(pUnit);
        }

        pBuf = BUFFER_MGR_Allocate();

        if (pBuf == NULL)
        {
            return nRet;
        }

        if (FS_RETURN_OK == FS_ReadFile(nUnit, (UI8_T *)pFile, pBuf, SYS_ADPT_MAX_FILE_SIZE, &read_count))
        {
            nRet = 1;
            cgi_SendHeader(http_connection, sock, read_count, envcfg);
            cgi_SendBin(http_connection, sock, read_count, pBuf);
            cgi_response_end(http_connection); // <-- +
        }

        BUFFER_MGR_Free(pBuf);
    }

    return nRet;
}

/* ----------------------------------------------------------------------
 * cgi_dispatch: Sends Web File.

 * Input:
 * sock: Socket identifier.
 * auth: Authorisation level: 1=guest, 2=administrator.
 * envcfg: Environment configuration.
 * envQuery: Query parameters.
 * ---------------------------------------------------------------------- */

void cgi_dispatch(HTTP_Event_T *event)
{
    HTTP_Connection_T  *http_connection = (HTTP_Connection_T *)event->data;
    int                 sock = http_connection->req->fd;
    int                 auth = http_connection->req->access_privilege;
    envcfg_t           *envcfg = http_connection->req->envcfg;
    envcfg_t           *envQuery = http_connection->req->query;

    int nRet;
    int bHaveSent = 0;

    /* do predispatch work */
    nRet = cgi_disp_before(http_connection, sock, auth, envcfg, envQuery);

    switch (nRet)
    {
        case 1:
            bHaveSent = 1;
            goto After;

        case 0:
            break;

        case -1:
            goto Exit;
    }

    nRet = cgi_disp_file(http_connection, sock, auth, envcfg, envQuery);

    /* special case, for Web download, send file context to broswer
     */
    if (0 == nRet)
    {
        nRet = cgi_disp_fixedWebDownload(http_connection, sock, auth, envcfg, envQuery);
    }

    if (nRet)
    {
        bHaveSent = 1;
    }

After:
    /* do postdispatch work */
    nRet = cgi_disp_after(http_connection, sock, auth, envcfg, envQuery);

    switch (nRet)
    {
        case 1:
            bHaveSent = 1;
            break;

        case 0:
            break;

        case -1:
            goto Exit;
    }

    /* not found */
    if (bHaveSent == 0)
    {
 //
 // TODO: Use con to replace nSock
 //
        cgi_notfound(http_connection, sock);
        bHaveSent = 1;
    }

Exit:

    ASSERT(/* the normal cases */
           (bHaveSent == 0 && http_connection->done == 0) ||
           (bHaveSent == 1 && http_connection->done == 1) ||
           /* the long pooling case */
           (bHaveSent == 1 && http_connection->done == 0));

    if (http_connection->wait_syscb)
    {
        event->event_type = HTTP_EVENT_READ;
        event->fd = http_connection->fds[HTTP_CONN_FD_NET];
        event->handler = cgi_main_end;
        event->data = http_connection;

        ASSERT(http_connection->done == 0);
        http_event_add(http_connection->worker->event_ctx, event, HTTP_TIMER_INFINITE);
    }
    else
    {
        event->event_type = HTTP_EVENT_READ;
        event->fd = http_connection->fds[HTTP_CONN_FD_NET];
        event->handler = cgi_main_finalize;
        event->data = http_connection;

        ASSERT(http_connection->done == 1);
        http_event_add(http_connection->worker->event_ctx, event, 0);
    }
}
