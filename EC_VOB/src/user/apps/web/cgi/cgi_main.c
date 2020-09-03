/* ----------------------------------------------------------------------
 * FILE NAME: cgi\cg_main.c

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Module for Fileless CGI.
 * This is the main program.

 * CLASSES AND FUNCTIONS:

 * Basic services:

 * cgi_streqi: Private version of "string equal, case-insensitive".
 * cgi_stresc: Convert string to escaped form.
 * cgi_SendText: Send to socket a text string.
 * cgi_SendHex: Send to socket a hexidecimal buffer converted to binary.
 * cgi_SendBin: Send to socket a binary buffer.

 * Send HTTP header:

 * cgi_SendHeader: Send to socket a URI header.
 * cgi_build_header: Build a header for the desired URI.
 * cgi_check_mime_map: Convert file-name extension into content-type.

 * Error replies:

 * cgi_notfound: File not found, send error reply.

 * Only for outside interface:

 * cgiMain: Main callback function from CGIC library.

 * HISTORY:
 * 1997-09-24 (Wed): Created by Daniel K. Chung.
 * 1997-10-01 (Wed): Has interface to CGIC library.
 * 1997-10-02 (Thu): Sends HTTP header.
 * 1997-10-06 (Mon): Checks authorisation level.
 * 1997-10-14 (Tue): Does not use global variables.
 * 1997-10-15 (Wed): Added "cgi_notfound".
 * 1997-10-16 (Thu): Added global constants.
 * 1997-10-20 (Mon): Uses new version of "rs_api.h".
 * 1997-10-24 (Fri): Passes authorisation level to CGI.
 * 1997-10-28 (Tue): If guest and admin are same, consider as admin.
 * 1997-10-28 (Tue): Added "cgi_stresc".
 * 1998-04-09 (Thu): Added "cgi_SendBin".
 * 1998-04-10 (Fri): Use #define to support ACCTON and ODS versions.
 * 1998-05-13 (Wed): Define OEM strings in "cgoem.h".
 * 1998-07-30 (Thu): Removed sprintf (causes problems with PowerPC).
 * 1998-07-31 (Fri): Converted all large arrays on stack to malloc.
 * 1998-11-09 (Mon): Tell browser not to cache.
 * 1998-11-27 (Fri): Removed code which stores unnecessary env.
 * 1999-04-27 (Tue): Porting to Geine platform by Sumei Chung.
 * 2006-03-08 (Wed): Checks file extension is XML, Brian.
 *
 * COPYRIGHT (C) Accton Technology Corporation, 1997.
 * ---------------------------------------------------------------------- */
#include "sys_bld.h"
#include "l_mm.h"
#include "cgi.h"
#include "cgi_request.h"
#include "cgi_response.h"
#include "cgi_auth.h"
#include "cgi_main.h"
#include "cgi_multi.h"
#include "cgi_cache.h"
#include "cgi_file.h"
#include "cgi_backdoor.h"
#include "cgi_module.h"
#include "cgi_rest.h"
#include "cgi_sem.h"
#include "cgi_util.h"
#include "backdoor_mgr.h"
#include "buffer_mgr.h"

/* ----------------------------------------------------------------------
 * Types.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * Constants.
 * ---------------------------------------------------------------------- */

#define BUFSIZE                 1024
#define MAX_FILEEXT_SIZE        32      /* maximum size of a filename extension */
#define MAX_FIELD_LEN           256     /* maximum length of header fields */
#define MAX_HEADER_LEN          2048    /* this should probably be same as TCP buf size */
#define MAX_URI_LEN             256     /* maximum length of a URI */

/* ----------------------------------------------------------------------
 * Global constants.
 * ---------------------------------------------------------------------- */

/* error string to be displayed in text field */

const char *cgi_szError = "Error";
const char *cgi_szNotPresent = "Not Present";

/* error to be inserted into "select" in HTML file */

const char *cgi_szErrorOption = "<option selected>Error</option>";

/* ----------------------------------------------------------------------
 * In-file constants.
 * ---------------------------------------------------------------------- */

/* mime mapping (copied from "http_srv\httproot.c") */

static const char *mime_file [] = {
    "htm", "html", "css", "js", "json",
    "gif", "jpg",
    "xml", "txt", "c", "h", "java", "jva",
    NULL
};

/* add text/xml, 2005-10-04
 */
static const char *mime_content [] = {
    "text/html", "text/html", "text/css", "text/plain", "text/plain",
    "image/gif", "image/jpg",
    "text/xml", "text/plain", "text/plain", "text/plain", "text/plain", "text/plain",
    NULL
};


#define CGI_MAX_NUMBER_TRY_SEND     100

static CGI_REST_CONTEXT_PTR_T cgi_main_api_ctx_ptr;

/* ----------------------------------------------------------------------
 * Forward declarations.
 * ---------------------------------------------------------------------- */

static void cgi_build_header(HTTP_Connection_T *http_connection, int nSock, char *header, envcfg_t *envcfg, int nLen);
static void cgi_check_mime_map(char *header, const char *path);

/* ----------------------------------------------------------------------
 * CGI_MAIN_InitiateSystemResources: Initialize system resources.
 * ---------------------------------------------------------------------- */
BOOL_T CGI_MAIN_InitiateSystemResources (void)
{
    return CGI_SEM_Create();
}

/* ----------------------------------------------------------------------
 * CGI_MAIN_Create_InterCSC_Relation: Callback for creating inter-CSC
 *                                    relationships.
 * ---------------------------------------------------------------------- */
void CGI_MAIN_Create_InterCSC_Relation (void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("cgi",
        SYS_BLD_WEB_GROUP_IPCMSGQ_KEY, CGI_BACKDOOR_CallBack);
}

/* ----------------------------------------------------------------------
 * CGI_MAIN_EnterMasterMode: This function forces this subsystem enter
 *                           the Master Operation mode.
 * ---------------------------------------------------------------------- */
void CGI_MAIN_EnterMasterMode (void)
{
    if (cgi_main_api_ctx_ptr)
    {
        cgi_rest_free_ctx(&cgi_main_api_ctx_ptr);
    }

    cgi_main_api_ctx_ptr = cgi_rest_new_ctx();

    CGI_MODULE_Init();
}

/* ----------------------------------------------------------------------
 * CGI_MAIN_EnterTransitionMode: This function forces this subsystem enter
 *                               the Transition Operation mode.
 * ---------------------------------------------------------------------- */
void CGI_MAIN_EnterTransitionMode (void)
{
    CGI_FILE_CacheClear();

    if (cgi_main_api_ctx_ptr)
    {
        cgi_rest_free_ctx(&cgi_main_api_ctx_ptr);
    }
}

/* ----------------------------------------------------------------------
 * CGI_MAIN_EnterSlaveMode: This function forces this subsystem enter
 *                          the Slave Operation mode.
 * ---------------------------------------------------------------------- */
void CGI_MAIN_EnterSlaveMode (void)
{
}

BOOL_T CGI_MAIN_Register(const char *path, CGI_API_HANDLER_SET_T *handlers, UI32_T flags)
{
    // cgi_rest_add_route(self, path, handler, flags)
    //  1. find path in api.json for schema
    //  2. create entry in llist, and send pointer to api.json

    return cgi_rest_add_routes(cgi_main_api_ctx_ptr, path, handlers, flags);
}

/* ----------------------------------------------------------------------
 * cgi_streqi: Private version of "string equal, case-insensitive".

 * As of now, there is no known function "strcmpi".

 * Input:
 * s1, s2: Pointer to two strings to be compared.

 * Output:
 * 0: Unequal.
 * non-zero: Equal.
 * ---------------------------------------------------------------------- */

int cgi_streqi (I8_T *s1, I8_T *s2)
{
    I8_T *p1 = s1, *p2 = s2;

    while (1)
    {
        /* whole strings equal */
        if ((*p1 == 0) && (*p2 == 0))
        {
            return 1;
        }

        /* if character not equal (including end-of-string), return */
        if (toupper (*p1) != toupper (*p2))
        {
            return 0;
        }

        /* next character */
        p1++;
        p2++;
    }
}

/* ----------------------------------------------------------------------
 * cgi_stresc: Convert string to HTML escaped form.

 * Input:
 * szOut, szIn: Pointer to output and input strings.
 * ---------------------------------------------------------------------- */

void cgi_stresc (char *szOut, const char *szIn)
{
    char *pOut = szOut;
    const char *pIn = szIn;
    char tmpbuf[10];

    while (*pIn != 0)
    {
        /* check special character */
        switch (*pIn)
        {
            case '&': strcpy (pOut, "&amp;"); pOut += 5; break;
            case '<': strcpy (pOut, "&lt;"); pOut += 4; break;
            case '>': strcpy (pOut, "&gt;"); pOut += 4; break;
            case '"': strcpy (pOut, "&quot;"); pOut += 6; break;

            default:
                if ((*pIn == '\'') || (*pIn == '\\') || (*pIn >= 128))
                {
                    /* nike 1998-7-30 sprintf (pOut, "&#%03d;", *pIn); pOut += 6;
                     */
                    strcpy (pOut, "&#");
                    cgi_convertDec2Str((UI32_T)*pIn, tmpbuf, 10);     /* nike 1998-7-30*/
                    switch(strlen(tmpbuf))
                    {
                        case 0:
                            strcat (pOut, "000");
                            break;
                        case 1:
                            strcat (pOut, "00");
                            break;
                        case 2:
                            strcat (pOut, "0");
                    }

                    strcat (pOut, tmpbuf);
                    strcat (pOut, ";");
                    pOut += 6;
                }
                else
                {
                    *pOut = *pIn; pOut++; break;
                }
        }

        pIn++;
    }

    *pOut = 0;
}

static int cgi_SendText1(HTTP_Connection_T *http_connection, int nSock, const char *szText);
static int cgi_SendBin1(HTTP_Connection_T *http_connection, int nSock, int nLen, const unsigned char *pBin);

int cgi_SendText(HTTP_Connection_T *http_connection, int nSock, const char *szText)
{
    if (NULL == http_connection)
    {
        return -1;
    }

    if (http_connection->res->headers_out.chunked)
    {
        return cgi_SendTextChunk1(http_connection, nSock, szText);
    }

    return cgi_SendText1(http_connection, nSock, szText);
}

/* ----------------------------------------------------------------------
 * cgi_SendText: Send to socket a text string.

 * Input:
 * nSock: Socket identifier.
 * szText: Pointer to text string.

 * Return:
 * 0: Success.
 * -1: Failure.
 * ---------------------------------------------------------------------- */
 //
 // TODO: Use res to replace nSock
 //
static int cgi_SendText1(HTTP_Connection_T *http_connection, int nSock, const char *szText)
{
    return cgi_SendBin1(http_connection, nSock, strlen(szText), (unsigned char *) szText);
}

int cgi_SendBin(HTTP_Connection_T *http_connection, int nSock, int nLen, const unsigned char *pBin)
{
    if (NULL == http_connection)
    {
        return -1;
    }

    if (http_connection->res->headers_out.chunked)
    {
        return cgi_SendBinChunk1(http_connection, nSock, nLen, pBin);
    }

    return cgi_SendBin1(http_connection, nSock, nLen, pBin);
}

/* ----------------------------------------------------------------------
 * cgi_SendBin1: Send to socket a binary buffer.

 * Input:
 * nSock: Socket identifier.
 * nLen: Length of binary buffer.
 * szHex: Pointer to binary buffer.
 * ---------------------------------------------------------------------- */
 //
 // TODO: Use http_connection to replace nSock
 //
static int cgi_SendBin1(HTTP_Connection_T *http_connection, int nSock, int nLen, const unsigned char *pBin)
{
    int nSegmentLen;
    const unsigned char *pStart = pBin;
    int nCount = nLen + 1; /* Preventing the infinite looping. In worst case,
                            * (the size of res->send_buf equal 1 and remain
                            * equal 0 at the first time)
                            * it will need (nLen + 1) times to send the text out.
                            */

    if (NULL == http_connection)
    {
        return -1;
    }

    while (0 < nLen && 0 < nCount--)
    {
        int remain = sizeof(http_connection->res->send_buf) - http_connection->res->send_len;

        ASSERT(0 <= remain);

        nSegmentLen = (nLen > remain) ? remain : nLen;

        memcpy(http_connection->res->send_buf + http_connection->res->send_len, pStart, nSegmentLen);
        http_connection->res->send_len += nSegmentLen;

        nLen -= nSegmentLen;
        pStart += nSegmentLen;

        if (0 < nLen)
        {
            if (cgi_SendBuffer(http_connection, nSock))
            {
                return -1;
            }
        }
    }

    return 0;
}

int cgi_SendTextChunk1(HTTP_Connection_T *http_connection, int nSock, const char *szText)
{
    return cgi_SendBinChunk1(http_connection, nSock, strlen(szText), (const unsigned char *)szText);
}

int cgi_SendBinChunk1(HTTP_Connection_T *http_connection, int nSock, int nLen, const unsigned char *pBin)
{
#define CRLF "\r\n"

    char chunk_line[100];

    /* 0 means EoF. Skip this.
     */
    if (nLen == 0)
    {
        return 0;
    }

    int l = snprintf(chunk_line, sizeof(chunk_line), "%x; %d" CRLF, nLen, nLen);
    cgi_SendBin1(http_connection, nSock, l, (unsigned char *) chunk_line);

    cgi_SendBin1(http_connection, nSock, nLen, pBin);
    cgi_SendBin1(http_connection, nSock, sizeof(CRLF) - 1, (unsigned char *) CRLF);

    return 0;
#undef CRLF
}

int cgi_SendEoFChunk1(HTTP_Connection_T *http_connection, int nSock)
{
#define CRLF "\r\n"

    cgi_SendBin1(http_connection, nSock, 3, (unsigned char *) "0" CRLF);
    cgi_SendBin1(http_connection, nSock, sizeof(CRLF) - 1, (unsigned char *) CRLF);
    return 0;
#undef CRLF
}

/* ----------------------------------------------------------------------
 * cgi_SendHeader: Send to socket a URI header.

 * Available for interface with Harrison Yeh (Ye Youxiong).

 * Input:
 * nLen: Length of URI content, -1 to omit length field.
 * ---------------------------------------------------------------------- */

void cgi_SendHeader(HTTP_Connection_T *http_connection, int nSock, int nLen, envcfg_t *envcfg)
{
    char *szHeader;

// TODO: allocate a header_out in response ??
    /* allocate buffer */
    if ((szHeader = (char *) malloc (MAX_HEADER_LEN)) == 0)
    {
        return;
    }
    MM_alloc(szHeader, MAX_HEADER_LEN, SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

    /* send header */
    cgi_build_header(http_connection, nSock, szHeader, envcfg, nLen);
    cgi_SendText1(http_connection, nSock, szHeader);

/* CgiExit: */
    /* free buffer */
    MM_free(szHeader, SYSFUN_TaskIdSelf());
    free (szHeader);
}

/* ----------------------------------------------------------------------
 * cgi_build_header: resolve MIME type, then
 *                   build HTTP header for the desired URI.
 * Input:
 *     nLen: Length of URI content.
 * ---------------------------------------------------------------------- */
static void cgi_build_header (HTTP_Connection_T *http_connection, int nSock, char *header, envcfg_t *envcfg, int nLen)
{
    char  *buffer;
    char  *session_id;
    char  *pValue;
#ifdef CGI_SPRINTF
    UI32_T  date, time, ticks;
#endif

    ASSERT(http_connection != NULL);

    /* allocate buffer */
    if ((buffer = (char *) L_MM_Malloc (MAX_FIELD_LEN, L_MM_USER_ID2(SYS_MODULE_CGI, CGI_TYPE_TRACE_ID_CGI_SUBMIT_ALLOCATE_TMPBUF))) == 0)
    {
        header [0] = '\0';
        return;
    }

    buffer[0] = '\0';

#ifdef CGI_SPRINTF
    sprintf (header, "HTTP/1.1 200 OK\r\n");
#else
    strcpy (header, "HTTP/1.1 200 OK\r\n");
#endif
    http_connection->res->headers_out.major_version = 1;
    http_connection->res->headers_out.major_version = 1;
    http_connection->res->headers_out.http_status = 200;

    if ((session_id = get_env(envcfg, "SESSION_ID")) != NULL)
    {
        const UI32_T MAX_UI32_STR_LEN = sizeof("4294967295") - 1;

        UI32_T max_age =
    #ifdef SYS_ADPT_HTTP_WEB_USER_CONNECTION_TIMEOUT
                SYS_ADPT_HTTP_WEB_USER_CONNECTION_TIMEOUT / SYS_BLD_TICKS_PER_SECOND;
    #else
                HTTP_WEB_USER_CONNECTION_TIMEOUT;
    #endif
        char sz_max_age[MAX_UI32_STR_LEN + 1];

        SYSFUN_Snprintf(sz_max_age, sizeof(sz_max_age), "%u", max_age);
        sz_max_age[sizeof(sz_max_age)-1] = '\0';

        strcat (header, "Set-Cookie: " HTTP_COOKIE_SESSION_ID "=");
        strcat (header, session_id);
        strcat (header, "; path=/");
        strcat (header, "; max-age=");
        strcat (header, sz_max_age);
        strcat (header, "\r\n");

        {
            if (http_connection->res->headers_out.cookies_n)
            {
                json_decref(http_connection->res->headers_out.cookies_n);
            }

            http_connection->res->headers_out.cookies_n = json_object();
            if (!http_connection->res->headers_out.cookies_n)
            {
                // TODO: log here !!
            }

            if (http_connection->res->headers_out.cookies_n)
            {
                json_t *cookies_n = http_connection->res->headers_out.cookies_n;

                json_object_set_new(cookies_n, HTTP_COOKIE_SESSION_ID, json_string(session_id));
                json_object_set_new(cookies_n, "path", json_string("/"));
                json_object_set_new(cookies_n, "max-age", json_string(sz_max_age));

                http_connection->res->headers_out.set_cookie = 1;
            }
        }
    }

#ifdef CGI_SPRINTF
    if (tm_get(&date, &time, &ticks))
    printf("HTPD: error trying to read time for building header\n");
    sprintf(buffer, "Date: %d %s %d %d:%02d:%02d GMT\r\n", (date & 0xff), month[(date >> 8) & 0xff],
    (date >> 16) & 0xffff,(time >> 16) & 0xffff,
    (time >> 8) & 0xff, time & 0xff);
    strcat(header, buffer);    /* copy Date into header */
    buffer[0] = '\0';
#endif

    /* Force browser not to cache the virtual documents(.htm), however,
       let it auto cache image(.gif) & script(.js) files */
    if (nLen == -1)
    {
        strcat (header, "Pragma: no-cache\r\n");
        strcat (header, "Expires: 0\r\n");

        http_connection->res->headers_out.no_cache = 1;
    }
    else
    {
        strcat (header, "Expires: Thu, 31 Dec 2020 00:00:00 GMT\r\n");
    }

    if (nLen != -1)
    {
        strcat(header, "Content-Length: ");
        /* nike 1998-7-30 sprintf(buffer, "%d\r\n", nLen);*/
        cgi_convertDec2Str((UI32_T)nLen, buffer, 10);     /* nike 1998-7-30*/
        strcat (buffer, "\r\n");
        strcat(header, buffer);
        buffer[0] = '\0';

        http_connection->res->headers_out.content_length = 1;
        http_connection->res->headers_out.content_length_n = nLen;
    }
    else
    {
        if (http_connection->res->transfer_encoding == HTTP_TRANSFER_ENCODING_CHUNKED)
        {
            strcat(header, "Transfer-Encoding: chunked\r\n");
//          strcat(header, buffer);

            http_connection->res->headers_out.chunked = 1;
        }
    }

    /* check file extension, then
       append the adequate "Content-Type: "<type/subtype>", based on req->request_uri */
    strcat (header, "Content-Type: ");
    {
        char *path_info = get_env(envcfg, "PATH_INFO");

        // TODO: check does we need this !
        pValue = get_env(envcfg, "CHANGE_CONTENT_TYPE");

        if (pValue != NULL)
        {
            strcat(header, pValue);
        }
        else
        {
            cgi_check_mime_map (header, path_info ? path_info : "");

            http_connection->res->headers_out.content_type = 1;

            /* Set attachment filename
             */
            {
                char *uri = get_env(envcfg, "URI");

                ASSERT(uri != NULL);

                if (uri != NULL)
                {
                    if (strncmp("/file/", uri, 6) == 0)
                    {
                        strcat (header,"Content-Disposition: attachment; filename=");
                        strcat (header, &uri[6]);
                        strcat (header, "\r\n");
                    }
                }
            }
        }
    }
    strcat(header, "\r\n");

    /* free buffer */
    L_MM_Free (buffer);
}

/* ----------------------------------------------------------------------
 * cgi_check_mime_map: Convert file-name extension into content-type.

 Name:   check_mime_map
 Description:  Try to determine the content-type by comparing the file
 extension of the file requested, with the mime mapping table.
 The default is "application/octet-stream" for no file extension,
 or no listing in the mime mapping table for a particular extension.
 * ---------------------------------------------------------------------- */
static void cgi_check_mime_map (char *header, const char *path)
{
    int i;
    const char *ext;
    const char *dflt_mime_type = "application/octet-stream\r\n";


    /* if no file (home page), default to "text/html" */
    if (strcmp (path, "/") == 0)
    {
        strcat (header, "text/html\r\n");
        return;
    }

    ext = CGI_FILE_GetFileExt(path);

    if (ext != NULL)
    {
        ext ++;
    }

    if (ext != NULL)
    {
        for (i = 0; mime_file[i] != NULL; ++ i)
        {
            if (!strcmp(mime_file[i], ext))
            {
                strcat (header, mime_content[i]);
                strcat (header, "\r\n");

                CGI_UTIL_SetXmlFlag( (!strcmp(ext, "xml")) ? TRUE : FALSE );

                return;
            }

        }
    }

    /* Not found in known MIME list
     */
    strcat(header, dflt_mime_type);
}

/* ----------------------------------------------------------------------
 * cgi_notfound: File not found, send error reply.

 * Input:
 * nSock: Socket identifier.
 * ---------------------------------------------------------------------- */

void cgi_notfound(HTTP_Connection_T *http_connection, int nSock)
{
    http_connection->req->status = R_NOT_FOUND;

    cgi_SendText1(http_connection, nSock, "HTTP/1.0 404 Not Found\r\n");
    cgi_SendText1(http_connection, nSock, "Content-Type: text/html\r\n\r\n");
    cgi_SendText1(http_connection, nSock, "<title>404 Not found</title><body>404 Not Found</body>");

    cgi_response_end(http_connection);
}

// TODO: rename to cgi_end ??
void cgi_response_end(HTTP_Connection_T *http_connection)
{
    if (http_connection->res->is_send_eof == 0)
    {
        if (http_connection->res->headers_out.chunked)
        {
            cgi_SendEoFChunk1(http_connection, http_connection->res->fd);
        }

        http_connection->res->is_send_eof = 1;
        http_connection->done = 1;

        if (!http_connection->res->headers_out.chunked)
        {
            http_connection->keepalive = 0;
        }
    }
}

/////////////////////////////////////////////////////////////////////
static int send_buffer(struct HTTP_Response_ST *resp, char *in, size_t in_len)
{
    HTTP_Connection_T *http_connection = resp->connection;

    return cgi_SendBin(http_connection, resp->fd, in_len, (unsigned char *)in);
}
/////////////////////////////////////////////////////////////////////

/* ----------------------------------------------------------------------
 * cgi_main: Main function of CGI library.

 * Input:
 * HTTP_Connection_T *http_connection.
 * ---------------------------------------------------------------------- */
void cgi_main(HTTP_Event_T *event)
{
    char *param_p;

    HTTP_TYPE_RESP_STATUS_T ret = HTTP_TYPE_RESP_DONE;

    HTTP_Connection_T *http_connection = (HTTP_Connection_T*) event->data;

    ASSERT(http_connection);
    ASSERT(http_connection->req);
    ASSERT(http_connection->req->envcfg);
    ASSERT(http_connection->req->query);

    CGI_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_CONNECTION, 1,
        "Process request, fd(%d)",
        http_connection->fds[HTTP_CONN_FD_NET]);

    ASSERT(http_connection->req->user_ctx == NULL);
    http_connection->req->user_ctx = CGI_REST_InternalNewRequestUserContext();
    if (http_connection->req->user_ctx == NULL)
    {
        ret = HTTP_TYPE_RESP_FAIL;
        goto fatal_error;
    }

    http_connection->res->write_response = send_buffer;

    ASSERT(http_connection->res->user_ctx == NULL);
    http_connection->res->user_ctx = CGI_REST_InternalNewResponseUserContext();
    if (http_connection->res->user_ctx == NULL)
    {
        ret = HTTP_TYPE_RESP_FAIL;
        goto fatal_error;
    }

    /* We only have query-string in GET, or body in POST.
     */
    param_p = get_env(http_connection->req->envcfg, "QUERY_STRING");
    if (param_p == NULL)
    {
        param_p = get_env(http_connection->req->envcfg, "BODY");
    }

    if (param_p != NULL)
    {
        CGI_QUERY_ParseQueryString(param_p, CGI_QUERY_InsertToEnv, http_connection->req->envcfg);
        CGI_QUERY_ParseQueryString(param_p, CGI_QUERY_InsertToEnv_Without_Dot_Query, http_connection->req->query);
    }

    http_connection->req->access_privilege = cgi_check_auth(http_connection, http_connection->req, http_connection->req->query);

    if (http_connection->req->access_privilege < 0)
    {
        ASSERT(http_connection->done == 1);
    }

    if (0 <= http_connection->req->access_privilege)
    {
        // invoke rest api here !!
        {
            /*const*/ CGI_REST_ROUTE_PTR_T route = cgi_rest_find_route(cgi_main_api_ctx_ptr, http_connection->req->request_uri, http_connection->req->method);

            if (route)
            {
                CGI_STATUS_CODE_T status_code = cgi_rest_call_route_handler(cgi_main_api_ctx_ptr, http_connection, route);

                cgi_rest_write_response(http_connection, status_code);

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
                return;
                ////////////////////////////////////////////////////////////////
            }
        }

        if (http_connection->req->multipart == 1)
        {
            // TODO: Check does need to call cgi_dispatch ??
            if (http_connection->req->access_privilege < CGI_HTTP_UPLOAD_PRIVILEGE_REQUEST)
            {
                set_env(http_connection->req->query, "page", "fileHttpNoPrivilege");

                event->handler = cgi_dispatch;
                cgi_dispatch(event);
                return;
            }
            else
            {
                event->handler = CGI_MULTI_AsyncUploadFile;
                CGI_MULTI_AsyncUploadFile(event);
                // FIXME: miss a return here ??
            }
        }
        else
        {
            event->handler = cgi_dispatch;
            cgi_dispatch(event);
            return;
        }
    }

fatal_error:
    event->handler = cgi_main_finalize;
    cgi_main_finalize(event);
}

void cgi_main_finalize(HTTP_Event_T *event)
{
    HTTP_Connection_T *http_connection = (HTTP_Connection_T*) event->data;

    http_connection_dbg_check(http_connection);

    cgi_SendBuffer(http_connection, http_connection->req->fd);

    if (http_connection->done == 1)
    {
        cgi_main_close(http_connection);
    }

    event->handler = http_request_finalize;
    http_request_finalize(event);
}

void cgi_main_end(HTTP_Event_T *event)
{
    HTTP_Connection_T *http_connection = (HTTP_Connection_T*) event->data;

    http_connection_dbg_check(http_connection);

    http_connection->done = 1;
    http_connection->keepalive = 0;
    http_connection->res->is_send_eof = 1;

    event->handler = cgi_main_finalize;
    cgi_main_finalize(event);
}

void cgi_main_close(HTTP_Connection_T *http_connection)
{
    HTTP_Request_T *req = http_connection->req;
    HTTP_Response_T *res = http_connection->res;

    // TODO: Add a HTTP_LOG_LEVEL_DEBUG --> to stdout in log, so we shall not need add
    //       the compiler option everyhere
#if (HTTP_CFG_CGI_DEBUG == 1)
    CGI_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_EVENT, 1,
                  "Free resource, fd(%d)",
                  http_connection->fds[HTTP_CONN_FD_NET]);
#endif /* HTTP_CFG_EVENT_DEBUG */

    if (req)
    {
        if (req->multipart_data)
        {
            BUFFER_MGR_Free((UI8_T *)req->multipart_data);
            req->multipart_data = NULL;
            req->multipart_len = 0;
        }

        if (req->boundary)
        {
            L_MM_Free(req->boundary);
            req->boundary = NULL;
            req->boundary_len = 0;
        }

        CGI_REST_InternalDeleteRequestUserContext((CGI_REQUEST_USER_CONTEXT_T **)&req->user_ctx);
    }

    if (res)
    {
        CGI_REST_InternalDeleteResponseUserContext((CGI_RESPONSE_USER_CONTEXT_T **)&res->user_ctx);
    }
}

/*
 * cgi_SendBuffer: send socket buffer
 * Return:
 * true or false
 */
 //
 // TODO: Use res to replace nSock
 //
int cgi_SendBuffer(HTTP_Connection_T *http_connection, int nSock)
{
    int rc;
    int sendNumber=0;
    char *send_pointer;
    BOOL_T isSend = FALSE;

    ASSERT(http_connection != NULL);
    if (http_connection == NULL)
    {
        return -1;
    }

    if (http_connection->res->send_len == 0)
    {
        return 0;
    }

    send_pointer = http_connection->res->send_buf;

send_packet:

    rc = HTTP_UTIL_Write(http_connection, nSock, send_pointer, http_connection->res->send_len, 0);

    switch (rc)
    {
    case -1:
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            sendNumber++;

            if (sendNumber <= CGI_MAX_NUMBER_TRY_SEND)
            {
                SYSFUN_Sleep(10);
                goto send_packet;
            }
            else
            {
                if (isSend == TRUE)
                    return -1;
            }
        }
        else
        {
#ifdef CGI_SPRINTF
            printf("\r\n %s %d Error on sending socket buffer. error no.=%d", __FUNCTION__, __LINE__, errno);
#endif
            return -1;
        }
        break;

        //
        // FIXME: Miss case zero returned ?
        //

    default:

        ASSERT(0 < rc);

        isSend = TRUE;
        if ((UI32_T) rc <= http_connection->res->send_len)
        {
            http_connection->res->send_len -= rc;
            if (http_connection->res->send_len > 0)
            {
                send_pointer+=rc;
                goto send_packet;
            }
        }
        else
        {
#ifdef CGI_SPRINTF
            printf("\n %s %d rc > send_len, rc=%d, send_len=%lu", __FUNCTION__, __LINE__,rc, send_len );
#endif
            return -1;
        }
        break;
    }

    http_connection->res->send_len = 0;
    memset(http_connection->res->send_buf, 0, sizeof(http_connection->res->send_buf));

    return 0;
}

