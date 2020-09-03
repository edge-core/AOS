/*****************************************************************************
;
;   (C) Unpublished Work of Accton Technology,  Corp.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF ACCTON TECHNOLOGY CORP.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) ACCTON EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN ACCTON WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF ACCTON.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;------------------------------------------------------------------------------
;
;    Project : ES3514G/ES3526G
;    Creator : Sumei Chung
;    File    : cg_multi.c
;    Abstract: Process multipart/form-data file upload in HTML
;
;Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;
;*****************************************************************************/

#include <string.h>
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_module.h"
#include "sysfun.h"
#include "l_mm.h"
#include "cgi.h"
#include "cgi_auth.h"
#include "cgi_main.h"

#include "buffer_mgr.h"

#include "http_util.h"
#include "http_connection.h" // TODO: move to cgi.h

#define CR          13      /* ASCII value of CR */
#define LF          10      /* ASCII value of LF */
#define SYN         34      /* ASCII value of SYN (")*/
#define BB          45      /* ASCII value of - */

#define CGI_MULTI_MAX_NAME_LEN  63
#define CGI_MULTI_MAX_VALUE_LEN 63

static UI32_T http_upgrade_status = CGI_TYPE_HTTP_UPGRADE_NOT_COPY;

static UI32_T cgi_multi_fileUpload (HTTP_Connection_T *http_connection,
                                    int in_sock, envcfg_t *env,
                                    char *data_P,
                                    int len,
                                    char *bdry_P,
                                    int bdry_len);

static void cgi_multipart_process(HTTP_Event_T *event);
static void cgi_multipart_headers(HTTP_Connection_T *http_connection);
static void cgi_multipart_read(HTTP_Connection_T *http_connection);

/**----------------------------------------------------------------------
 * To set HTTP upgrade flag
 *
 * @param  status   The value of status
 * @return          TRUE means success; elsewise FALSE
 * ---------------------------------------------------------------------- */
BOOL_T CGI_MULTI_SetHttpUpgradeFlag(UI32_T status)
{
    UI32_T orig_priority, http_om_semid;

    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_HTTP_OM, &http_om_semid);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(http_om_semid);

    if (CGI_TYPE_HTTP_UPGRADE_COPYING == status)
    {
        if (CGI_TYPE_HTTP_UPGRADE_COPYING == http_upgrade_status)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(http_om_semid, orig_priority);
            return FALSE;
        }
    }

    http_upgrade_status = status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(http_om_semid, orig_priority);
    return TRUE;
}

/**----------------------------------------------------------------------
 * To get HTTP upgrade flag
 *
 * @param  status   The value of status will be one of the following
 *                  CGI_TYPE_HTTP_UPGRADE_NOT_COPY, CGI_TYPE_HTTP_UPGRADE_COPYING
 * ---------------------------------------------------------------------- */
void CGI_MULTI_GetHttpUpgradeFlag(UI32_T *status)
{
    UI32_T orig_priority, http_om_semid;

    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_HTTP_OM, &http_om_semid);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(http_om_semid);
    *status = http_upgrade_status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(http_om_semid, orig_priority);
}

/**----------------------------------------------------------------------
 * To process HTTP upgrade
 *
 * @param  http_connection   http_connection object
 * ---------------------------------------------------------------------- */
void CGI_MULTI_UploadFileMain(HTTP_Event_T *event)
{
    char *value_p;
    int  length;

    HTTP_Connection_T *http_connection = (HTTP_Connection_T *)event->data;

    if (NULL == (value_p = get_env(http_connection->req->envcfg,(char *)"CONTENT_LENGTH")))
    {
        http_connection->req->status = R_BAD_REQUEST;
        goto final;
    }

    length = atoi(value_p);

    if (SYS_ADPT_MAX_FILE_SIZE < length)
    {
        http_connection->req->status = R_BAD_REQUEST;
        goto final;
    }

    http_connection->req->multipart_data = (char *) BUFFER_MGR_Allocate();
    if (http_connection->req->multipart_data == NULL)
    {
        http_connection->req->status = R_ERROR; // FIXME: show a html to explain
                                                //        why have this error
        goto final;
    }

    memcpy(http_connection->req->multipart_data,
           &http_connection->req->header_in[http_connection->req->body_index],
           http_connection->req->body_cnt);
    http_connection->req->multipart_len = http_connection->req->body_cnt;

    http_connection->req->multipart_parsed_ptr = http_connection->req->multipart_data;

    HTTP_MGR_Check_User_Connection_Info(http_connection->req);

    cgi_multipart_headers(http_connection);

    if (400 <= http_connection->req->status)
    {
        goto final;
    }

    event->handler = cgi_multipart_process;
    http_event_add(http_connection->worker->event_ctx, event, 0);
    return;

final:
    CGI_MULTI_SetHttpUpgradeFlag(CGI_TYPE_HTTP_UPGRADE_NOT_COPY);
    http_connection->done = 1;
    return;
}

/**----------------------------------------------------------------------
 * To spawn a thread to do HTTP upgrade
 *
 * @param  http_connection   http_connection object
 * @return              status code
 * ---------------------------------------------------------------------- */
void CGI_MULTI_AsyncUploadFile(HTTP_Event_T *event)
{
    UI32_T  status;
    HTTP_Connection_T *http_connection = (HTTP_Connection_T *)event->data;

    if (http_connection == NULL)
    {
        ASSERT(0);
        return;
    }

    http_connection_dbg_check(http_connection);

    CGI_MULTI_GetHttpUpgradeFlag(&status);

    /* there is another multipart in process
     */
    if (CGI_TYPE_HTTP_UPGRADE_COPYING == status)
    {
        http_connection->req->status = R_SILENT; // FIXME: NEED A HTML FOR THIS ERROR
        http_connection->done = 1;
        return;
    }
    CGI_MULTI_SetHttpUpgradeFlag(CGI_TYPE_HTTP_UPGRADE_COPYING);

    set_env(http_connection->req->envcfg, "FILE_UPLOAD_FLAG", "succeed");

    event->handler = CGI_MULTI_UploadFileMain;
    CGI_MULTI_UploadFileMain(event);
}

/**----------------------------------------------------------------------
 * Get field's name start address.
 *
 * @param  addr_P       pointer for searching string
 * @param  total_len    searching string length
 * @return              the length for field's name address
 * @note                return 0, if cannot find any "name=" string
 * ---------------------------------------------------------------------- */
static int cgi_multi_getNameStartAddress (char *addr_P, int total_len)
{
    int i;
    char szName[] = "name=";
    int szNameLen = sizeof(szName)-1;

    for (i= 0 ; i < total_len; i++)
    {
        if (memcmp (addr_P + i, szName, szNameLen) == 0)
        {
            // got NAME attribute pointer
            return i + szNameLen;
        }
    }
    return 0;
}

/**----------------------------------------------------------------------
 * search through data to determine value's start address
 * the value is start by a CRLFCRLF
 *
 * @param  start_P
 * @param  len
 * @return
 * ---------------------------------------------------------------------- */
static int cgi_multi_checkStartOfValue (char *start_P,  int len)
{
    int i = 0;


    for (i = 0; i < len; i++)
    {
        if (start_P [i] == CR &&
            start_P [i+1] == LF &&
            start_P [i+2] == CR &&
            start_P [i+3] == LF)
    {
        return i + 4 ;
    }
    }
    return 0;
}

/**----------------------------------------------------------------------
 *
 *
 * @param  chk_P
 * @param  pkt_len
 * @param  bdry_P
 * @param  bdry_len
 * @return
 * ---------------------------------------------------------------------- */
static int cgi_multi_checkValueLength (char *chk_P,  int pkt_len, char *bdry_P, int bdry_len)
{
    int i, chk_len;

    chk_len = pkt_len - bdry_len - 4;

    for (i = 0; i <= chk_len; i++)
    {
        if (chk_P [i] == CR &&
            chk_P [i+1] == LF &&
            chk_P [i+2] == '-' &&
            chk_P [i+3] == '-' &&
            memcmp (chk_P + (i+4), bdry_P, bdry_len) == 0)
        {
            return i;
        }
    }
    return -1;
}

/**----------------------------------------------------------------------
 *
 *
 * @param  in_sock
 * @param  env
 * @param  data_P
 * @param  len
 * @param  bdry_P
 * @param  bdry_len
 * @return
 * ---------------------------------------------------------------------- */
static UI32_T cgi_multi_fileUpload (HTTP_Connection_T *http_connection,
                                    int in_sock, envcfg_t *env,
                                    char *data_P,
                                    int len,
                                    char *bdry_P,
                                    int bdry_len)
{
    UI32_T nTotalLen = 0;
    char   *addr_P;
    int    i;
    char   *pStoreAll;
    int    nRet = 0;
    int    recv_len=0;
    char   szTemp [32];

    pStoreAll = data_P;
    recv_len = len;

    /* address to store next packet */
    addr_P = pStoreAll + nTotalLen;

    if (len < bdry_len)
    {
        return 2;
    }

    if ((i = cgi_multi_checkValueLength (data_P, len, bdry_P, bdry_len)) >= 0 )
    {
        len = i;
    }
    else
    {
        return 2;
    }

    /* store buffer */
    memcpy (addr_P, data_P, len);
    nTotalLen += len;

    /* Zhong Qiyao, Foxfire, 1999-07-19 */
    /* store address and length */
    sprintf (szTemp, "%p", pStoreAll);
    set_env (env, "DOWNLOAD_ADDRESS", szTemp);
    sprintf (szTemp, "%lu", (unsigned long) nTotalLen);
    set_env (env, "DOWNLOAD_LENGTH", szTemp);
    return (nRet);
}

/**----------------------------------------------------------------------
 * Parse the receive packets
 *
 * @param  sock
 * @param  envQuery
 * @param  envcfg
 * @param  firstPkt_P
 * @param  firstPkt_len
 * ---------------------------------------------------------------------- */
void cgi_multipart(HTTP_Connection_T *http_connection, int sock, envcfg_t *envQuery, envcfg_t *envcfg, char *firstPkt_P, int firstPkt_len)
{
    char     *bdry_P, *start_P;
    int      bdry_len;
    int      i, chk_len, len, temp_len;
    char     nameBuf[CGI_MULTI_MAX_NAME_LEN + 1];
    char     valueBuf[CGI_MULTI_MAX_VALUE_LEN + 1];

    bdry_P = http_connection->req->boundary;
    bdry_len = http_connection->req->boundary_len;

    // parse receive packet
    // --boundary<CR><LF>!#@$@$name=<SYN>NAME1<SYN><CR><LF><CR><LF>VALUE1<CR><LF>--boundary
    // @$#@$name=<SYN>UPLOAD_FILEAME<SYN>; UPLOAD_FILENAME="file"<CR><LF><CR><LF>VALUE2<CR><LF>--boundary--


    if (firstPkt_len < bdry_len)
    {
        http_connection->req->status = R_BAD_REQUEST;
        return;
    }
    else
    {
        chk_len = firstPkt_len - bdry_len ;
        start_P = &firstPkt_P [bdry_len];
    }

    while (chk_len > 0)
    {
        len = cgi_multi_getNameStartAddress (start_P, chk_len);

        // if can not get string "name=", we need to recv next packet form the nSock
        if (len == 0)
        {
            http_connection->req->status = R_BAD_REQUEST;
            return;
        }
        else
        {
            chk_len -= (len + 1);
        }
        start_P += (len + 1);

        if (chk_len > 0)
        {
            // copy name
            for (i = 0; *start_P != SYN && i <= _countof(nameBuf);)
            {
                chk_len--;
                if (chk_len == 0)
                {
                    http_connection->req->status = R_BAD_REQUEST;
                    return;
                }

                // overflow
                if (_countof(nameBuf) <= i)
                {
                    break;
                }

                nameBuf[i++] = *start_P;
                ++start_P;
            }

            // last space is used for null terminal
            if (_countof(nameBuf) <= i)
            {
                continue;
            }

            ASSERT(i < sizeof(nameBuf));
            nameBuf[i] = 0;
        }

        // get value start address

        len =  cgi_multi_checkStartOfValue (start_P, chk_len);
        /* if can not get value start address, we need to recv next packet form the nSock */
        if (len == 0)
        {
            http_connection->req->status = R_BAD_REQUEST;
            return;
        }
        else
        {
            chk_len -= len;
        }
        start_P += len;

        if (strcmp (nameBuf, "webUploadSrcFile") == 0)
        {
            // debug ! ! !
            // do check in this section
            if (0) // debug
            {
                const char *p = firstPkt_P;
                const char find[] = "name=\"webUploadSrcFile\"; filename=\"";

                while (1)
                {
                    if (memcmp(p, find, sizeof(find) - 1) == 0)
                    {
                        char filename[100];

                        p += (sizeof(find) - 1);

                        len = strcspn(p, ".");
                        memcpy(filename, p, len);

                        ASSERT(len <= sizeof(filename));
                        filename[len] = '\0';

                        set_env (envcfg, "webUploadSrcFileFileName", filename);
                        break;
                    }

                    ++p;
                }
            }

            // process file upload procedure......
            /* fail */
            if (cgi_multi_fileUpload (http_connection, sock, envcfg, start_P, chk_len, bdry_P, bdry_len) != 0)
            {
                set_env (envcfg, "FILE_UPLOAD", "fail");
            }
            else
            {
                set_env (envcfg, "FILE_UPLOAD", "succeed");
            }
            goto END;
        }

        temp_len = 0;

        if (chk_len > 0)
        {
            while ( (len = cgi_multi_checkValueLength (start_P, chk_len, bdry_P, bdry_len)) < 0 )
            {
                http_connection->req->status = R_BAD_REQUEST;
                return;
            }

            if (sizeof(valueBuf) <= (temp_len + len))
            {
                start_P += len;
                chk_len -=len;

                continue;
            }

            ASSERT((temp_len + len) < sizeof(valueBuf));
            memcpy (&valueBuf [temp_len], start_P, len);
            valueBuf[temp_len + len] = '\0';

            start_P += len;
            chk_len -=len;
        }
        else
        {
            http_connection->req->status = R_BAD_REQUEST;
            return;
        }

        set_env (envQuery, nameBuf, valueBuf);
    }

END:
    return;
}

static void cgi_multipart_process(HTTP_Event_T *event)
{
    HTTP_Connection_T  *http_connection = (HTTP_Connection_T *)event->data;
    const char         *str_content_length;
    size_t              content_length;

    str_content_length = get_env(http_connection->req->envcfg, "CONTENT_LENGTH");

    ASSERT(str_content_length);

    content_length = atol(str_content_length);
    if (http_connection->req->multipart_len < content_length)
    {
        cgi_multipart_read(http_connection);

        if (400 <= http_connection->req->status)
        {
            goto final;
        }

        if (http_connection->req->multipart_len < content_length)
        {
            event->handler = cgi_multipart_process;
            http_event_add(http_connection->worker->event_ctx, event, HTTP_TIMER_INFINITE);
            return;
        }
    }

    /* All date had been recived
     */
    cgi_multipart(http_connection,
                  (int) http_connection->req->fd,
                  http_connection->req->query,
                  http_connection->req->envcfg,
                  http_connection->req->multipart_data,
                  http_connection->req->multipart_len);

    if (400 <= http_connection->req->status)
    {
        goto final;
    }

    event->handler = cgi_dispatch;
    cgi_dispatch(event);
    return;

final:
    CGI_MULTI_SetHttpUpgradeFlag(CGI_TYPE_HTTP_UPGRADE_NOT_COPY);

    http_connection->done = 1;

    event->handler = cgi_main_finalize;
    cgi_main_finalize(event);
    return;
}

static void cgi_multipart_headers(HTTP_Connection_T *http_connection)
{
    char   *content_type;
    size_t  i;
    size_t  content_type_len;

    content_type = get_env(http_connection->req->envcfg, "CONTENT_TYPE");
    if (content_type == NULL)
    {
        http_connection->req->status = R_BAD_REQUEST;
        return;
    }

    for (i = 0, content_type_len = strlen(content_type); i < content_type_len; i++)
    {
        if (content_type[i] == '=')
        {
            http_connection->req->boundary_len = strlen(content_type) - i - 1;
            http_connection->req->boundary = (char *) L_MM_Malloc(http_connection->req->boundary_len,
                                                                  L_MM_USER_ID2(SYS_MODULE_CGI, CGI_TYPE_TRACE_ID_CGI_MULTIPART_BDRY));

            if (http_connection->req->boundary == NULL)
            {
                http_connection->req->status = R_ERROR;
                return;
            }

            memcpy(http_connection->req->boundary, &content_type[i+1], http_connection->req->boundary_len);
            break;
        }
    }
}

static void cgi_multipart_read(HTTP_Connection_T *http_connection)
{
    const UI32_T  max_len = SYS_ADPT_MAX_FILE_SIZE;

    int     len;

    len = HTTP_UTIL_Read(http_connection,
                         http_connection->fds[HTTP_CONN_FD_NET],
                         &http_connection->req->multipart_data[http_connection->req->multipart_len],
                         max_len - http_connection->req->multipart_len,
                         0);
    if (len <= 0)
    {
        http_connection->req->status = R_SILENT;
        return;
    }

    http_connection->req->multipart_len += len;
}
